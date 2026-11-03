# Compatibility

Current source: **0.1.2-audit**, 2026-11-03. No configure, compilation, DB or runtime test was performed
for this revision. Static inspection is not runtime compatibility certification.

| Source / mode | Evidence |
| --- | --- |
| Official Playerbot core `93333d6a9eb00fdf0179adde30da3c17618207d6` + Playerbots `05a1f6fceb8a32aa879b13f5f48934c6e46a774c` | 0.1.1 historical x64 Release static worldserver compile/link passed; 0.1.2 source/API inspection only |
| Same pair without Commander | Historical baseline full worldserver compile/link passed |
| Original local integrated archive | Read-only feature baseline; its precise core commit is unknown |
| Reference LLM repository `45ac63a5b7a4182df22873f20b7834ca3afb3b9e` | GitHub HEAD verified on audit date; 38 LlmBots files compared against local baseline |
| VS 2026 / MSVC 19.51 / Ninja | Historical 0.1.1 toolchain; current source uncompiled |
| VS 2022, Linux, other commits | Unverified |
| Dynamic Commander / Playerbots DLLs | Unsupported; explicit CMake rejection |
| Playerbots with embedded LlmBots scripts | Unsupported; duplicate .llmbot registration even if disabled |

The late CMake hook checks required headers, including the observed world-thread login-operation
interfaces, and selects manager AI lookup or the observed GET_PLAYERBOT_AI fallback. Typed assertions
check AI, preset and login/logout interfaces when the user compiles. No CMake check was executed in this audit.
The existing macro fallback has no verified build evidence.

To extend support, inspect the candidate version's actual headers, add an adapter under src/Compat,
retain explicit failure for unknown structures, and validate both baseline and combined static builds
plus owner login/logout, tool permissions and game commands in an isolated server. Do not select behavior
by commit hash or bypass API checks with casts/private-memory access. Core and Playerbots remain external.
