#Requires -Version 5.1
[CmdletBinding()]
param([Parameter(Mandatory = $true)][string] $WorkRoot)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'
. (Join-Path $PSScriptRoot '..\tools\windows\LLM-SETUP.ps1')

function Assert-True {
    param([bool] $Condition, [string] $Message)
    if (!$Condition) { throw $Message }
}

function Get-TestHash {
    param([string] $Path)
    return (Get-FileHash -LiteralPath $Path -Algorithm SHA256).Hash
}

function New-TestServer {
    param([string] $Name, [string] $Text, [Text.Encoding] $Encoding)
    $root = Join-Path $WorkRoot $Name
    if (Test-Path -LiteralPath $root) { throw 'Test directory already exists; no fixtures are overwritten/deleted.' }
    [void](New-Item -ItemType Directory -Path (Join-Path $root 'configs\modules'))
    # Inert marker only. The test never executes any EXE or contacts a DB.
    [IO.File]::WriteAllBytes((Join-Path $root 'worldserver.exe'), [byte[]]@(0))
    $template = Join-Path $root 'configs\modules\mod_llm_commander.conf.dist'
    [IO.File]::WriteAllText($template, $Text, $Encoding)
    return Get-SetupPaths $root
}

$WorkRoot = [IO.Path]::GetFullPath($WorkRoot)
if ($WorkRoot -notmatch '(?i)^C:\\module\\Setup-Dev\\validation\\') {
    throw 'Fixture writes must stay under C:\module\Setup-Dev\validation.'
}
if (Test-Path -LiteralPath $WorkRoot) { throw 'Use a new test directory; tests do not delete old fixtures.' }
[void](New-Item -ItemType Directory -Path $WorkRoot)
$utf8 = New-Object Text.UTF8Encoding($false)
$secretText = [guid]::NewGuid().ToString('N')
$secret = ConvertTo-SecureString $secretText -AsPlainText -Force
$passed = New-Object Collections.Generic.List[string]
try {
    $template = "# preserved`r`nLlmBots.Openai.ApiKey = `"`"`r`nLlmCommander.Openai.ApiKey = @legacy@`r`nLlmBots.Enable = 0`r`n"
    $paths = New-TestServer 'new-config' $template $utf8
    $templateHash = Get-TestHash $paths.Template
    $result = Set-SetupApiKey $paths $secret
    Assert-True ($result.ChangedKey -eq 'LlmBots.Openai.ApiKey') 'Canonical key not selected'
    Assert-True ($null -eq $result.Backup) 'New configuration must not invent a backup'
    Assert-True ((Get-TestHash $paths.Template) -eq $templateHash) 'Template changed'
    $expected = $template.Replace('LlmBots.Openai.ApiKey = ""', 'LlmBots.Openai.ApiKey = "' + $secretText + '"')
    Assert-True ((Read-SetupConfig $paths.Config).Text -ceq $expected) 'Other config text changed'
    $passed.Add('new config: canonical key, template and unrelated values preserved')

    $existing = "# existing`nLlmBots.Openai.ApiKey = keep-canonical`nLlmCommander.Openai.ApiKey = keep-alias`nLlmBots.Commander.AccountId = 321`n"
    $paths = New-TestServer 'existing-utf16' $template ([Text.Encoding]::Unicode)
    [IO.File]::WriteAllText($paths.Config, $existing, [Text.Encoding]::Unicode)
    $beforeHash = Get-TestHash $paths.Config
    $result = Set-SetupApiKey $paths $secret
    Assert-True ($result.ChangedKey -eq 'LlmCommander.Openai.ApiKey') 'Existing effective alias ignored'
    Assert-True ((Get-TestHash $result.Backup) -eq $beforeHash) 'Backup is not byte-identical'
    $bytes = [IO.File]::ReadAllBytes($paths.Config)
    Assert-True ($bytes[0] -eq 255 -and $bytes[1] -eq 254) 'UTF-16 BOM not preserved'
    $expected = $existing.Replace('keep-alias', '"' + $secretText + '"')
    Assert-True ((Read-SetupConfig $paths.Config).Text -ceq $expected) 'Existing unrelated values changed'
    Assert-True ((Get-Acl -LiteralPath $paths.Config).AreAccessRulesProtected) 'Config ACL is not private'
    Assert-True ((Get-Acl -LiteralPath $result.Backup).AreAccessRulesProtected) 'Backup ACL is not private'
    $passed.Add('existing alias: byte-identical backup, encoding, values and private ACL preserved')

    $paths = New-TestServer 'duplicates' "LlmBots.Openai.ApiKey = first`nLlmBots.Openai.ApiKey = second`n" $utf8
    $beforeHash = Get-TestHash $paths.Template
    $rejected = $false
    try { Set-SetupApiKey $paths $secret | Out-Null } catch { $rejected = $true }
    Assert-True $rejected 'Duplicate configuration was accepted'
    Assert-True (!(Test-Path -LiteralPath $paths.Config)) 'Failure created an active config'
    Assert-True ((Get-TestHash $paths.Template) -eq $beforeHash) 'Failure changed template'
    $passed.Add('duplicate-key refusal leaves existing files intact')

    foreach ($invalid in @("bad`nvalue", 'bad"value', 'bad value')) {
        $paths = New-TestServer ('invalid-' + [guid]::NewGuid().ToString('N')) $template $utf8
        $bad = ConvertTo-SecureString $invalid -AsPlainText -Force
        $rejected = $false
        try { Set-SetupApiKey $paths $bad | Out-Null } catch { $rejected = $true } finally { $bad.Dispose() }
        Assert-True $rejected 'Config injection value was accepted'
        Assert-True (!(Test-Path -LiteralPath $paths.Config)) 'Rejected input created config'
    }
    $passed.Add('newline, quote and whitespace injection refused')

    foreach ($path in @('relative\server', 'C:\azerothcore-w\Acore-Portable', 'C:\azerothcore-w\Acore-Release')) {
        $rejected = $false
        try { Assert-SetupLocalPath $path | Out-Null } catch { $rejected = $true }
        Assert-True $rejected 'Protected/relative path accepted'
    }
    $passed.Add('protected and relative paths refused before filesystem access')

    $paths = New-TestServer 'check-only' $template $utf8
    $beforeHash = Get-TestHash $paths.Template
    $output = Invoke-LlmSetup -Root $paths.Root -ReadOnly 6>&1 | Out-String
    Assert-True (!(Test-Path -LiteralPath $paths.Config)) 'Read-only mode wrote config'
    Assert-True ((Get-TestHash $paths.Template) -eq $beforeHash) 'Read-only mode changed template'
    Assert-True (!$output.Contains($secretText)) 'Secret was printed'
    $passed.Add('read-only mode writes nothing and prints no secret')

    $guide = Show-SetupAccountGuide 6>&1 | Out-String
    Assert-True ($guide.Contains('.playerbots account setKey')) 'Native setKey guidance missing'
    Assert-True ($guide.Contains('.playerbots account link LLMBOTS')) 'Native link guidance missing'
    Assert-True (!$guide.Contains($secretText)) 'Guide leaked an API key'
    $passed.Add('native account guidance uses placeholders and no secrets')
    [pscustomobject]@{ Passed = $passed.Count; Checks = @($passed.ToArray()); NoServerOrDatabaseAccess = $true } |
        ConvertTo-Json -Depth 3
} finally { $secret.Dispose(); $secretText = $null }
