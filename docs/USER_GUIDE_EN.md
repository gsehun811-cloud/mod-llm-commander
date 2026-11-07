# LLM Commander — English User Guide

> **For:** AzerothCore WotLK 3.3.5a + standalone `mod-llm-commander` + compatible `mod-playerbots`.
>
> Based on this repository's configuration template, C++ command handlers, prompt builder, module implementation, and Characters DB schema. Source-level support does not guarantee a successful build or runtime behavior on every server. The repository's `0.1.2-audit` revision was not itself built or run as part of the audit.

[한국어 설명서 / Korean guide](USER_GUIDE_KO.md)

## Quick start

1. Build the standalone Commander module **statically** alongside a compatible Playerbots **without embedded `src/LlmBots` scripts**. The integrated LLM version also registers `.llmbot`, even when disabled. Setting `LlmBots.Enable=0` is **not enough** to resolve that conflict.
2. Install `mod_llm_commander.conf` and set the enabled flag, Commander bot name, authorized **human** account ID, and model IDs.
3. Set your OpenAI API key in the Windows environment variable `LLM_COMMANDER_OPENAI_API_KEY`.
4. Prepare an existing Playerbot character named `Cm`. From a GM character, run `.llmbot add Cm adventurer`.
5. Log in with the authorized human account, whisper `/w Cm hello`, then check `.llmbot stats` and your server log.

## 1. Commander vs. ordinary LLM bots

**Commander (CM)** is an existing Playerbot character identified by `LlmBots.Commander.Name`. Its command interface accepts whispers only from the human account configured by `LlmBots.Commander.AccountId`. It may use the specifically implemented Commander action tools, subject to account authorization, Playerbots permissions, and game-state checks.

**Ordinary LLM bots** can converse using their personas and read game-state information through their read-only tools. Combat, movement, and quest behavior still comes from Playerbots. Ordinary LLM bots do not receive Commander action tools.

**Personas** influence the LLM's responses through its system prompt. They do not alter the underlying Playerbots combat AI or grant extra permissions.

Important: assigning a Commander does **not** completely disable its normal Playerbots AI, movement, fighting, or acceptance of invitations. A tool being called successfully does not prove that a requested in-game action finished.

## 2. Configure the module

Create the active configuration file from `conf/mod_llm_commander.conf.dist`. Depending on your installation layout, the target is `server/configs/modules/mod_llm_commander.conf` or `configs/modules/mod_llm_commander.conf`.

Example:

```ini
LlmBots.Enable = 1
LlmBots.Provider = openai
LlmBots.Commander.Name = Cm
LlmBots.Commander.AccountId = 101
LlmBots.Openai.Endpoint = https://api.openai.com
LlmBots.Openai.ModelFast = "gpt-4.1-mini"
LlmBots.Openai.ModelThink = "gpt-4.1-mini"
LlmBots.MaxRosterSize = 20
LlmBots.MaxCallsPerMinute = 20
LlmBots.MaxTokensPerHour = 600000
LlmBots.Social.CooldownSec = 20
LlmBots.Plan.IntervalSec = 300
```

Replace the example account ID and model IDs with valid values for your setup.

| Option | What it means |
| --- | --- |
| `LlmBots.Enable` | Enable this LLM module |
| `LlmBots.Provider` | Select the provider (for example, `openai`) |
| `LlmBots.Commander.Name` | **Existing bot character name**, possibly normalized from `CM` to `Cm` |
| `LlmBots.Commander.AccountId` | **Human player's account ID**, **not** the bot's account; `0` denies Commander action access |
| `LlmBots.Openai.ModelFast` | Model for conversation/social tasks |
| `LlmBots.Openai.ModelThink` | Model for planning tasks |
| `LlmBots.MaxRosterSize` | Maximum registered LLM roster size |
| `LlmBots.MaxCallsPerMinute` | Soft rate limit |
| `LlmBots.MaxTokensPerHour` | Soft hourly token threshold, **not** a hard dollar-cost cap |
| `LlmBots.Social.CooldownSec` | Social interaction cooldown |
| `LlmBots.Plan.IntervalSec` | Periodic agenda interval; `0` disables periodic planning for ordinary roster bots |

`LlmBots.BotAccount` is retained metadata; it does **not** create accounts or characters automatically.

The module also accepts the legacy `LlmCommander.*` option aliases. A real alias value overrides the corresponding `LlmBots.*` value. The distributed `@legacy@` marker means “inherit”; do not replace it casually.

**Restart worldserver after changing configuration or environment variables.** `.llmbot reload` refreshes roster and personas only, not general configuration.

## 3. Windows: set the OpenAI API key

Open **PowerShell** as the Windows user who launches the server. To persist the key for that user:

```powershell
[Environment]::SetEnvironmentVariable(
    "LLM_COMMANDER_OPENAI_API_KEY",
    "YOUR_OPENAI_API_KEY_HERE",
    "User"
)
```

Open a **new** PowerShell session and check that the variable exists **without printing the secret**:

```powershell
[bool][Environment]::GetEnvironmentVariable(
    "LLM_COMMANDER_OPENAI_API_KEY",
    "User"
)
```

`True` means a value is saved. The `worldserver.exe` process must **inherit that variable**: restart any old console or launcher that still has an outdated environment, then restart worldserver. You do not necessarily need to reboot Windows.

Temporary, current-session-only setting:

```powershell
$env:LLM_COMMANDER_OPENAI_API_KEY = "YOUR_OPENAI_API_KEY_HERE"
```

The module also supports `LLM_COMMANDER_FOUNDRY_API_KEY`. Dedicated environment keys override values in the config file. Never commit actual secrets to GitHub, include them in a portable distribution, or paste them into public logs. An OpenAI API account is billed separately from a ChatGPT subscription.

### TLS certificate problems on Windows

If you encounter `certificate verify failed`, you may supply an **existing, trusted PEM CA bundle** via `LLM_COMMANDER_CA_FILE`.

```powershell
[Environment]::SetEnvironmentVariable(
    "LLM_COMMANDER_CA_FILE",
    "C:\azcorepo\server\ca-bundle.crt",
    "User"
)
```

That file path is **only an example**; make sure the file really exists and comes from a trustworthy source. Do **not** disable TLS verification. See [Windows CM troubleshooting](WINDOWS_CM_SETUP_KO.md) for a documented test case.

## 4. Set up the Commander character and account

Example only:

| Role | Example |
| --- | --- |
| Human account | `TEST` |
| Human account ID | `101` |
| Bot account | `LLMBOTS` |
| Commander character | `Cm` |

### Step A — find the human account ID

Query the **Auth DB**, not the Characters DB:

```sql
SELECT id, username
FROM acore_auth.account
WHERE username = 'TEST';
```

Use the returned ID as `LlmBots.Commander.AccountId`. **Do not use the bot account's ID.**

### Step B — create or prepare an existing Playerbot character

The Commander must be a character that already exists on an appropriate Playerbots account. `.llmbot add` neither creates accounts/characters nor guarantees their login.

If you manage a bot owned by a different account, Playerbots account linking **may** be required. Commands can vary across Playerbots versions.

Sign in with a character on the **bot** account:

```text
.playerbots account setKey YOUR_SEPARATE_LINK_KEY
```

Sign in with a character on the **human** account:

```text
.playerbots account link LLMBOTS YOUR_SEPARATE_LINK_KEY
.playerbots account linkedAccounts
.playerbots bot add Cm
```

The account-link key is **not** the OpenAI API key or the WoW account password. Some setups will not require linking.

### Step C — register CM as an LLM roster character

Run these commands **in-game as an authorized GM**, not from the worldserver console:

```text
.llmbot add Cm adventurer
.llmbot list
.llmbot stats
```

`.llmbot list` confirms roster registration, not an API response or a successful bot login.

With the authorized human account logged into the world:

```text
/w Cm hello
/w Cm check the current group status
```

The module rechecks account authorization and delegates actionable commands to the supported Playerbots functionality. Natural-language instructions do **not** imply unlimited ability to control the game.

## 5. Register ordinary LLM bots and manage the roster

These are **in-game GM** commands:

```text
.llmbot add Alice scholar
.llmbot add Bob
.llmbot list
.llmbot persona Alice grump
.llmbot remove Bob
.llmbot reload
.llmbot stats
.llmbot say Alice hello there
.llmbot goal Alice
.llmbot pause
.llmbot resume
```

| Command | Behavior |
| --- | --- |
| `.llmbot list` | Show enabled roster entries |
| `.llmbot add <name> [persona]` | Register an **existing** character; if omitted, choose one of five built-in personas based on GUID |
| `.llmbot remove <name>` | Remove roster registration **without deleting the character** |
| `.llmbot persona <name> <personaId>` | Set an existing persona; an unknown persona ID is rejected |
| `.llmbot reload` | Reload roster and personas from the DB |
| `.llmbot stats` | Show in-memory budget/error statistics |
| `.llmbot say <name> <prompt...>` | Request one social response |
| `.llmbot goal <name>` | Request a planning response |
| `.llmbot pause / resume` | Stop/restart LLM work; does not stop the underlying Playerbots AI |

### Built-in personas

The module seeds these five persona IDs from `data/sql/characters/base/001_llm_commander.sql`:

| Persona ID | Character style |
| --- | --- |
| `adventurer` | Enthusiastic explorer who loves new places and scenery |
| `braggart` | Loud, boastful self-proclaimed hero who exaggerates achievements |
| `scholar` | Dry, analytical researcher who keeps explaining things during combat |
| `grump` | Grumbling retired soldier who secretly cares for others |
| `theorist` | Thinker who suspects something is wrong with death, resurrection, and repeating reality |

To inspect **what is actually installed** on your server:

```sql
SELECT persona, display_name, brief, voice, quirks, self_awareness
FROM acore_characters.llmc_bot_persona
ORDER BY persona;
```

## 6. Create your own persona (no C++ changes needed)

The standalone module stores personas in:

```text
acore_characters.llmc_bot_persona
```

**Do not confuse this with** the integrated original's `acore_playerbots.llm_bot_persona` table. Data does not migrate automatically between them.

`.llmbot add Alice You are a WoW player...` is **not** supported as a free-form prompt. The `persona` argument is a persona **ID**, not a sentence. First create that ID in the database; then select it in-game.

Example using a UTF-8/utf8mb4 SQL connection:

```sql
INSERT INTO acore_characters.llmc_bot_persona
(persona, display_name, brief, voice, quirks, self_awareness)
VALUES (
    'friendly_player',
    'Friendly WoW Player',
    'You are a player who enjoys World of Warcraft, adventuring with friends and chatting about what happens in the game.',
    'Use natural, relaxed conversational English.',
    'Likes discussing dungeons, gear, and funny encounters with party members.',
    0
);
```

This example **adds** a new ID. If `friendly_player` already exists, the insert will raise a duplicate-key error; check before inserting, and do not overwrite existing personas unintentionally.

After inserting:

```text
.llmbot reload
.llmbot add Alice friendly_player
```

If Alice is **already** registered, use:

```text
.llmbot persona Alice friendly_player
```

Persona fields:

| Field | Purpose |
| --- | --- |
| `persona` | The ID used by commands (schema maximum: 48 characters) |
| `display_name` | Friendly title |
| `brief` | Backstory and role |
| `voice` | Speaking style |
| `quirks` | Character habits |
| `self_awareness` | Degree of awareness that something about the world is strange; `0` means staying fully in character |

`src/Bots/CommanderPromptBuilder.cpp` feeds `brief`, `voice`, `quirks`, and `self_awareness` into the LLM system prompt. The output is still probabilistic and will not always follow the style perfectly.

## 7. Reduce API usage

Optional lighter configuration:

```ini
LlmBots.MaxCallsPerMinute = 3
LlmBots.MaxTokensPerHour = 10000
LlmBots.Plan.IntervalSec = 0
```

This suppresses periodic planning for ordinary LLM roster bots and lowers soft usage limits. It is **not** a guaranteed dollar budget cap; requests already in progress can exceed configured thresholds. Model selection also affects actual cost.

## 8. Troubleshooting

| Symptom | What to check |
| --- | --- |
| `No such character` | Existing character name and character cache |
| `not my bot` | Playerbots account ownership/linking |
| `Invalid security key` | Key configured on the **bot account** being linked |
| Roster added but bot does not log in | Human owner account actually in world, Playerbots permissions/login, watchdog |
| No response / only fallback text | Module enabled, API key inherited by worldserver, model IDs, `.llmbot stats`, `Server.log` |
| `certificate verify failed` | Valid trusted PEM bundle and `LLM_COMMANDER_CA_FILE` |
| Persona not found | Correct `acore_characters.llmc_bot_persona` ID and `.llmbot reload` |

See [Windows CM setup and TLS troubleshooting](WINDOWS_CM_SETUP_KO.md) and [installation notes](INSTALL_KO.md).

## 9. Compatibility and known limits

- Standalone Commander **must not** coexist with Playerbots' integrated `LlmBots` command implementation.
- The standalone module uses **Characters DB `llmc_*` tables**; the integrated original used **Playerbots DB `llm_bot_*` tables**. There is no automatic migration.
- The intended supported build is **static**, not dynamic DLL linking.
- Ordinary LLM bots use read-only LLM tools; only the configured Commander has Commander action tools, with authorization checks.
- `0.1.2-audit` is a source-audit/restoration revision, not a promise that the current revision has been built and tested end-to-end.

Repository: [mod-llm-commander](https://github.com/gsehun811-cloud/mod-llm-commander) · [Korean user guide](USER_GUIDE_KO.md) · [Windows CM setup](WINDOWS_CM_SETUP_KO.md).
