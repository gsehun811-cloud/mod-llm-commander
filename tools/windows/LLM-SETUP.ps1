#Requires -Version 5.1
# GPL-2.0-or-later. Local settings only; no server execution or database access.
[CmdletBinding()]
param(
    [string] $ServerRoot,
    [switch] $Check,
    [switch] $AccountGuideOnly
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

function Assert-SetupLocalPath {
    param([string] $Path)
    if ($Path -notmatch '^[A-Za-z]:[\\/]' -or $Path -match '[\r\n]') {
        throw 'Use a fully qualified local Windows path.'
    }
    $full = [IO.Path]::GetFullPath($Path)
    if ($full -match '(?i)^C:\\azerothcore-w\\Acore-(Portable|Release)(\\|$)') {
        throw 'This existing deployment is outside the setup scope.'
    }
    $cursor = $full
    while ($cursor) {
        if (Test-Path -LiteralPath $cursor) {
            $item = Get-Item -LiteralPath $cursor -Force
            if (($item.Attributes -band [IO.FileAttributes]::ReparsePoint) -ne 0) {
                throw 'Setup does not write through junctions or symbolic links.'
            }
        }
        $cursor = [IO.Path]::GetDirectoryName($cursor)
    }
    return $full
}

function Get-SetupPaths {
    param([string] $Root)
    $full = Assert-SetupLocalPath $Root
    $config = Assert-SetupLocalPath (Join-Path $full 'configs\modules\mod_llm_commander.conf')
    $template = Assert-SetupLocalPath ($config + '.dist')
    if (!(Test-Path -LiteralPath (Join-Path $full 'worldserver.exe') -PathType Leaf)) {
        throw 'Run setup beside worldserver.exe, or pass -ServerRoot with the installed server directory.'
    }
    if (!(Test-Path -LiteralPath $config -PathType Leaf) -and
        !(Test-Path -LiteralPath $template -PathType Leaf)) {
        throw 'The installed LLM Commander configuration/template is missing.'
    }
    return [pscustomobject]@{ Root = $full; Config = $config; Template = $template }
}

function Read-SetupConfig {
    param([string] $Path)
    $bytes = [IO.File]::ReadAllBytes($Path)
    $encoding = New-Object Text.UTF8Encoding($false, $true)
    $offset = 0
    if ($bytes.Length -ge 3 -and $bytes[0] -eq 239 -and $bytes[1] -eq 187 -and $bytes[2] -eq 191) {
        $encoding = New-Object Text.UTF8Encoding($true, $true)
        $offset = 3
    } elseif ($bytes.Length -ge 2 -and $bytes[0] -eq 255 -and $bytes[1] -eq 254) {
        $encoding = [Text.Encoding]::Unicode
        $offset = 2
    } elseif ($bytes.Length -ge 2 -and $bytes[0] -eq 254 -and $bytes[1] -eq 255) {
        $encoding = [Text.Encoding]::BigEndianUnicode
        $offset = 2
    }
    return [pscustomobject]@{
        Bytes = $bytes; Encoding = $encoding
        Text = $encoding.GetString($bytes, $offset, $bytes.Length - $offset)
    }
}

function Get-SetupEntry {
    param([string] $Text, [string] $Key)
    $pattern = '(?m)^[ \t]*' + [regex]::Escape($Key) + '[ \t]*=[ \t]*(?<value>[^\r\n]*)'
    $entries = [regex]::Matches($Text, $pattern)
    if ($entries.Count -gt 1) { throw 'Duplicate API-key setting found; resolve it manually before setup.' }
    if ($entries.Count -eq 1) {
        return [pscustomobject]@{
            Match = $entries[0]; Value = $entries[0].Groups['value'].Value.Trim().Replace('"', '')
        }
    }
    return $null
}

function Get-SetupApiTarget {
    param([string] $Text)
    $canonical = Get-SetupEntry $Text 'LlmBots.Openai.ApiKey'
    $alias = Get-SetupEntry $Text 'LlmCommander.Openai.ApiKey'
    if ($null -ne $alias -and $alias.Value -ne '@legacy@') {
        return [pscustomobject]@{ Key = 'LlmCommander.Openai.ApiKey'; Entry = $alias }
    }
    return [pscustomobject]@{ Key = 'LlmBots.Openai.ApiKey'; Entry = $canonical }
}

function Set-SetupPrivateAccess {
    param([string] $Path)
    $user = [Security.Principal.WindowsIdentity]::GetCurrent().User
    $acl = New-Object Security.AccessControl.FileSecurity
    $acl.SetOwner($user)
    $acl.SetAccessRuleProtection($true, $false)
    foreach ($sid in @($user.Value, 'S-1-5-18', 'S-1-5-32-544')) {
        $identity = New-Object Security.Principal.SecurityIdentifier($sid)
        $rule = New-Object Security.AccessControl.FileSystemAccessRule($identity, 'FullControl', 'Allow')
        $acl.AddAccessRule($rule)
    }
    Set-Acl -LiteralPath $Path -AclObject $acl
}

function New-SetupPrivateFile {
    param([string] $Path)
    $stream = [IO.File]::Open($Path, [IO.FileMode]::CreateNew, [IO.FileAccess]::Write, [IO.FileShare]::None)
    $stream.Dispose()
    Set-SetupPrivateAccess $Path
}

function Set-SetupApiKey {
    param($Paths, [Security.SecureString] $Secret)
    # Recheck paths at write time; neither existing credentials nor values are logged.
    [void](Get-SetupPaths $Paths.Root)
    $exists = Test-Path -LiteralPath $Paths.Config -PathType Leaf
    $inputPath = if ($exists) { $Paths.Config } else { $Paths.Template }
    $original = Read-SetupConfig $inputPath
    $target = Get-SetupApiTarget $original.Text
    $pointer = [IntPtr]::Zero
    try {
        $pointer = [Runtime.InteropServices.Marshal]::SecureStringToBSTR($Secret)
        $plain = [Runtime.InteropServices.Marshal]::PtrToStringBSTR($pointer)
        if ([string]::IsNullOrWhiteSpace($plain) -or $plain -match '[\s"#\\]') {
            throw 'The API key is empty or contains characters unsupported by the core config parser.'
        }
        $line = $target.Key + ' = "' + $plain + '"'
        if ($null -ne $target.Entry) {
            $match = $target.Entry.Match
            $text = $original.Text.Substring(0, $match.Index) + $line +
                $original.Text.Substring($match.Index + $match.Length)
        } else {
            $newline = if ($original.Text.Contains("`r`n")) { "`r`n" } else { "`n" }
            $text = $original.Text + $newline + $line + $newline
        }
        $temporary = $Paths.Config + '.setup-' + [guid]::NewGuid().ToString('N') + '.tmp'
        New-SetupPrivateFile $temporary
        [IO.File]::WriteAllText($temporary, $text, $original.Encoding)
        $backup = $null
        if ($exists) {
            $current = [Convert]::ToBase64String([IO.File]::ReadAllBytes($Paths.Config))
            if ($current -ne [Convert]::ToBase64String($original.Bytes)) {
                throw 'The configuration changed during setup; the existing file was left untouched.'
            }
            $backup = $Paths.Config + '.setup-' + [guid]::NewGuid().ToString('N') + '.bak'
            New-SetupPrivateFile $backup
            [IO.File]::WriteAllBytes($backup, $original.Bytes)
            Set-SetupPrivateAccess $Paths.Config
            [IO.File]::Replace($temporary, $Paths.Config, [System.Management.Automation.Language.NullString]::Value)
        } else {
            [IO.File]::Move($temporary, $Paths.Config)
        }
        return [pscustomobject]@{ Config = $Paths.Config; Backup = $backup; ChangedKey = $target.Key }
    } finally {
        if ($pointer -ne [IntPtr]::Zero) { [Runtime.InteropServices.Marshal]::ZeroFreeBSTR($pointer) }
        $plain = $null
        $text = $null
    }
}

function Show-SetupAccountGuide {
    Write-Host 'LLMBOTS password: preserve the existing account and password unless you intend to change them.'
    Write-Host 'If the account does NOT exist, enter this existing command in the server console yourself:'
    Write-Host '  account create LLMBOTS <your-password>'
    Write-Host 'Only when intentionally changing an EXISTING password:'
    Write-Host '  account set password LLMBOTS <new-password> <same-new-password>'
    Write-Host 'Playerbots security key: log in to a character on LLMBOTS and enter in game:'
    Write-Host '  .playerbots account setKey <your-security-key>'
    Write-Host 'Then log in on the owner account and enter in game:'
    Write-Host '  .playerbots account link LLMBOTS <the-same-security-key>'
    Write-Host 'Existing links and keys can be kept; do not reset them unnecessarily.'
    Write-Host 'Setup never captures these passwords/keys, sends commands, connects to a DB, or changes accounts.'
    Write-Host 'Enter secrets yourself in the existing server/client; avoid transcripts or shared screenshots.'
}

function Invoke-LlmSetup {
    param([string] $Root, [switch] $ReadOnly, [switch] $GuideOnly)
    $paths = Get-SetupPaths $Root
    if ($GuideOnly) { Show-SetupAccountGuide; return }
    Write-Host ('Server directory: ' + $paths.Root)
    Write-Host 'Existing config values are preserved. No server restart, DB update, download or build is performed.'
    $inputPath = if (Test-Path -LiteralPath $paths.Config) { $paths.Config } else { $paths.Template }
    $document = Read-SetupConfig $inputPath
    $target = Get-SetupApiTarget $document.Text
    $configured = $null -ne $target.Entry -and ![string]::IsNullOrWhiteSpace($target.Entry.Value)
    Write-Host ('API key in effective config: ' + $(if ($configured) { 'present (hidden)' } else { 'not set' }))
    $environmentKey = [Environment]::GetEnvironmentVariable('LLM_COMMANDER_OPENAI_API_KEY', 'Process')
    if (![string]::IsNullOrEmpty($environmentKey)) {
        Write-Host 'An API-key environment override is present (hidden). It is preserved and takes precedence.'
    }
    if ($ReadOnly) { Write-Host 'Read-only check complete; no files changed.'; return }
    if ([string]::IsNullOrEmpty($environmentKey)) {
        $choice = Read-Host 'Set/change the OpenAI key? Type SET to proceed; Enter keeps the existing settings'
        if ($choice -ceq 'SET') {
            $secret = Read-Host 'OpenAI API key (hidden)' -AsSecureString
            try {
                $result = Set-SetupApiKey $paths $secret
                Write-Host 'API-key setting saved. Other settings were preserved; a private backup was kept if applicable.'
            } finally { $secret.Dispose() }
        }
    }
    Show-SetupAccountGuide
    Write-Host 'Configure the existing Enable, Commander.Name, human AccountId and model settings as needed.'
    Write-Host 'Settings are read on the next normal server start. Setup does not start or stop the server.'
}

if ($MyInvocation.InvocationName -ne '.') {
    if (!$PSBoundParameters.ContainsKey('ServerRoot')) {
        $ServerRoot = Split-Path -LiteralPath $MyInvocation.MyCommand.Path
    }
    try { Invoke-LlmSetup -Root $ServerRoot -ReadOnly:$Check -GuideOnly:$AccountGuideOnly }
    catch {
        # Do not echo exception objects: they can contain user input or config contents.
        Write-Error 'LLM setup failed. Existing config/backup files were preserved. See LLM-SETUP-KO.md.'
        exit 1
    }
}
