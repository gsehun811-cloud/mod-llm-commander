# mod-llm-commander

Independent LLM Commander for AzerothCore WotLK + external mod-playerbots. Playerbots continues to
provide combat, movement and quest AI. GPL-2.0-or-later; see LICENSE, AUTHORS.md and upstream notices.
Vendored nlohmann/json is MIT licensed.

**0.1.2-audit: source compatibility restoration; NOT compiled or run.** The previous 0.1.1 build passed
on the official source pair listed in [Compatibility](docs/COMPATIBILITY.md). That result does not
validate these changes. This revision was inspected without configuring CMake, building, starting a
server, updating a DB or calling an external API.

## User guides / 사용자 설명서

- **[English user guide — Commander setup, ordinary LLM bots, personas, Windows API keys and troubleshooting](docs/USER_GUIDE_EN.md)**
- **[한국어 사용자 설명서 — CM 설정, 일반 LLM 봇, 페르소나, Windows 환경변수 및 문제 해결](docs/USER_GUIDE_KO.md)**
- [Windows CM setup and TLS troubleshooting / Windows CM 연결 및 TLS 문제 해결](docs/WINDOWS_CM_SETUP_KO.md)
- [한국어 설치 및 빌드 안내 / Korean installation notes](docs/INSTALL_KO.md)

## Install

1. Place this folder beside `modules/mod-playerbots` in a compatible AzerothCore source tree.
2. Use Playerbots **without embedded `src/LlmBots` scripts**. Both implementations register `.llmbot`
   even when disabled, so simultaneous installation is rejected. Turning off LlmBots.Enable is insufficient.
3. Configure/build your core with static modules. Dynamic Playerbots/Commander DLL linking is unsupported.
4. Prepare the Characters DB schema in `data/sql/characters/`. The inspected core's enabled module SQL
   updater can discover and apply these files at server startup; a CMake install only copies files.
   For deployments without source/updater discovery, apply the supplied SQL manually after backup.
5. Install `conf/mod_llm_commander.conf.dist` as `mod_llm_commander.conf` in the core's module config folder.
   Existing `mod_llm_bots.conf` **contents** may be copied unchanged into that registered filename.
6. Set `LlmBots.Enable`, `LlmBots.Commander.Name`, the human owner's `LlmBots.Commander.AccountId`, and
   model IDs. Provide API credentials in the server environment `LLM_COMMANDER_OPENAI_API_KEY`
   (or `LLM_COMMANDER_FOUNDRY_API_KEY`). Optional PEM trust bundle: `LLM_COMMANDER_CA_FILE`.
   Without that variable, an existing `ca-bundle.crt` in the working directory is loaded as in the original.
7. Create existing bot characters through Playerbots, then register them as an authorized GM:
   `.llmbot add Cm adventurer`. When the configured human account enters the world, the original
   roster auto-login/watchdog runs; it logs roster bots out when that account leaves.

Defaults: disabled, AccountId=0 denies Commander actions, Plan.IntervalSec=300 seconds.
The original BotAccount option is retained as metadata; neither version creates accounts with it.
No legacy DB data is migrated automatically. The original Playerbots DB `llm_bot_*` tables and these
Characters DB `llmc_*` tables are separate. Back up and explicitly migrate data if needed.

## Configuration compatibility

All 25 original `LlmBots.*` keys are supported. For existing standalone configurations, a real
`LlmCommander.*` value overrides the matching legacy key. In the distributed template `@legacy@`
means inherit the legacy value; an explicit empty value or 0 remains an override. A setting is read
once, never applied twice. The dedicated API-key environment variables override both prefixes.
Configuration is cached until restart; `.llmbot reload` reloads roster/personas only.
Install the updated `.conf.dist` as well as the source: ConfigMgr needs its alias declarations.

## Commands

All commands require `rbac::RBAC_PERM_COMMAND_GM` and an in-game session (`Console::No`).

| Command | Arguments | Effect |
| --- | --- | --- |
| `.llmbot list` | none | List enabled roster entries, as in the original |
| `.llmbot add` | name [persona] | Register an existing character |
| `.llmbot remove` | name | Remove roster row; never delete a character |
| `.llmbot persona` | name persona | Set an existing persona |
| `.llmbot reload` | none | Reload roster/personas |
| `.llmbot stats` | none | Show in-memory statistics |
| `.llmbot say` | name prompt... | Request dialogue |
| `.llmbot goal` | name | Request an agenda/narrative |
| `.llmbot pause` | none | Pause LLM work |
| `.llmbot resume` | none | Resume LLM work |

`.llmcommander` is no longer registered. Ordinary bots receive the original eight read-only tools;
the Commander receives the original eight Commander tools. Only the configured Commander Playerbot
can execute those tools for a currently online, non-bot character on the configured human account.
Native `playerbot_command` permission handling is delegated to Playerbots, as in the original.

OpenAI Responses retains function calling. Foundry retains conversation-only behavior; Mock retains
canned offline dialogue. Rate and hourly token limits are soft limits, not a hard currency budget.
Raid roles are session memory. Persistent usage writes and automatic long-term memory generation
were absent in the original and remain unimplemented.

The earlier requirement for ordinary bots to remain conversation-only is retained: agenda output
does not change ordinary bots' gameplay strategies. The Commander's manual goal retains its original
strategy toggles; its action tools are unchanged. Transport deadlines,
TLS verification, malformed-argument rejection, budget fixes and stale-completion guards are retained.
See [the complete audit](docs/COMPATIBILITY_AUDIT_KO.md) for all remaining differences and verification limits.

## Removal and verification

Disable/remove the module and reconfigure/rebuild the core. Keep module DB tables and configuration;
no destructive uninstall is provided. Module shutdown stops its workers; the restored owner-logout hook
uses the native Playerbots logout API. Source removal/rebuild was not performed in this audit.

The read-only audit tool requires only Python:

```text
python tools/audit_sources.py /path/to/original/mod-playerbots/src/LlmBots
```

It checks source inventories, command handlers, config keys and tool declarations. It does not compile
or prove in-game behavior. C++ offline test sources remain in `tests/` for a later user-run build.

[한국어 설치 안내](docs/INSTALL_KO.md) · [호환성 감사](docs/COMPATIBILITY_AUDIT_KO.md) ·
[문제 해결](docs/TROUBLESHOOTING_KO.md) · [배포 파일 목록](FILELIST.txt) · [Credits](AUTHORS.md)
