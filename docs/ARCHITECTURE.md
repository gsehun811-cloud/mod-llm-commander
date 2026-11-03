# Architecture

`src/Llm` retains providers, schemas, budget, worker queue and HTTPS transport without Playerbots types.
`src/Bots` retains brains, registry, conversation guard, chronicler and tools. Ordinary brains advertise
8 social tools and the Commander advertises the original 8 Commander tools. Agenda strategy changes
are blocked for ordinary bots. The Commander retains manual goal strategy toggles and its action tools.

`src/Script` registers the unique loader `Addmod_llm_commanderScripts`, PlayerScript, WorldScript and
original `.llmbot` command tree. Namespace/file/guard/loader names remain isolated. Because command and
legacy configuration names are shared, co-installation with embedded LlmBots is explicitly unsupported.

The world update drains completions and ticks brains after MapMgr's update/wait barrier. Workers
exchange strings and IDs; action execution revalidates Commander and the online human account.
Generation guards invalidate stale completions. Map-side events enter the locked event bus.
The restored owner login/logout hooks and 10-second watchdog delegate to Playerbots through
`src/Compat/CommanderPlayerbotsAdapter.cpp`; no Playerbots engine implementation is copied.

The 25 LlmBots options are original defaults. Registered LlmCommander aliases inherit through
`@legacy@` or override exactly once. Values are cached at startup; API-key environment overrides remain.
Module config filename stays `mod_llm_commander.conf` because the core's loader discovers that template.

Characters DB tables use `llmc_` to avoid sharing legacy Playerbots DB tables. SQL files are installed
as files; enabled core module SQL discovery can apply them at startup. No data migration is automatic.
Roster reads/writes are synchronous and use connection-aware escaping rather than patching core
prepared-statement enums. Long-term memory reads remain; automatic memory/usage writes are absent
in both compared versions. Raid roles are session-only.

HTTPS verifies TLS peers/hostnames, enforces an end-to-end deadline and caps response bodies at 2 MiB.
Request follow-ups/retries pass the budget gate. Token accounting remains a soft threshold, not a
currency cap. See COMPATIBILITY_AUDIT_KO.md for remaining semantic differences and unverified behavior.
