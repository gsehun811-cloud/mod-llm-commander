# 0.1.2-audit 변경 파일

2026-11-03 소스 감사 시작 시점의 SHA-256과 비교했습니다. Git 저장소 초기화는 하지 않았습니다.

## 동작 및 호환성 변경

- CommanderLlmBotsScript/Module: .llmbot과 원본 owner 로그인/로그아웃/watchdog 복원.
- CommanderPlayerbotsAdapter: 기존 공개 로그인 큐/로그아웃 API 위임 및 타입 검사 추가.
- CommanderLlmBotsConfig: 기존 25개 키와 기본값, 새 별칭 단일 우선순위, 불필요 clamp 제거.
- CommanderLlmToolbox: 원본 8+8 Tool 광고와 native 명령 권한 위임 복원.
- CommanderHttpsClient: 기존 ca-bundle.crt fallback 복원.
- 모듈 CMake 소스: embedded LlmBots 명령 충돌 거부, 로그인 API 헤더 존재 검사 추가. 구성 실행 없음.
- 원본 대응 src 38개: 저작권/배포 고지와 독립 수정 날짜 보존. 이 중 동작 변경이 없는 파일도 포함됨.
- SQL: .llmbot 명령 주석만 변경. 실행문/스키마/seed 데이터 변경 없음. 실제 DB 실행 없음.
- 문서/manifest/ignore/라이선스: 감사 결과와 과거 빌드 근거 분리, 배포 제외 규칙 보강.

## 수정 파일

- `.gitignore`
- `AUTHORS.md`
- `FILELIST.txt`
- `README.md`
- `SHA256SUMS`
- `VERSION`
- `conf/mod_llm_commander.conf.dist`
- `data/sql/characters/base/001_llm_commander.sql`
- `docs/ARCHITECTURE.md`
- `docs/COMPATIBILITY.md`
- `docs/INSTALL_KO.md`
- `docs/REPORT_KO.md`
- `docs/TROUBLESHOOTING_KO.md`
- `docs/VALIDATION.json`
- `mod-llm-commander.cmake`
- `src/Bots/CommanderChroniclerBot.cpp`
- `src/Bots/CommanderChroniclerBot.h`
- `src/Bots/CommanderLlmBotBrain.cpp`
- `src/Bots/CommanderLlmBotBrain.h`
- `src/Bots/CommanderLlmBotRegistry.cpp`
- `src/Bots/CommanderLlmBotRegistry.h`
- `src/Bots/CommanderLlmChatLog.cpp`
- `src/Bots/CommanderLlmChatLog.h`
- `src/Bots/CommanderLlmConversation.cpp`
- `src/Bots/CommanderLlmConversation.h`
- `src/Bots/CommanderLlmEventBus.cpp`
- `src/Bots/CommanderLlmEventBus.h`
- `src/Bots/CommanderLlmToolbox.cpp`
- `src/Bots/CommanderLlmToolbox.h`
- `src/Bots/CommanderPromptBuilder.cpp`
- `src/Bots/CommanderPromptBuilder.h`
- `src/CommanderLlmBotsConfig.cpp`
- `src/CommanderLlmBotsConfig.h`
- `src/Compat/CommanderPlayerbotsAdapter.cpp`
- `src/Compat/CommanderPlayerbotsAdapter.h`
- `src/Llm/CommanderFoundryProvider.cpp`
- `src/Llm/CommanderFoundryProvider.h`
- `src/Llm/CommanderHttpsClient.cpp`
- `src/Llm/CommanderHttpsClient.h`
- `src/Llm/CommanderLlmBudget.cpp`
- `src/Llm/CommanderLlmBudget.h`
- `src/Llm/CommanderLlmClient.cpp`
- `src/Llm/CommanderLlmClient.h`
- `src/Llm/CommanderLlmProvider.cpp`
- `src/Llm/CommanderLlmProvider.h`
- `src/Llm/CommanderLlmSchema.cpp`
- `src/Llm/CommanderLlmSchema.h`
- `src/Llm/CommanderLlmTypes.h`
- `src/Llm/CommanderMockProvider.cpp`
- `src/Llm/CommanderMockProvider.h`
- `src/Llm/CommanderOpenAiProvider.cpp`
- `src/Llm/CommanderOpenAiProvider.h`
- `src/Script/CommanderLlmBotsModule.cpp`
- `src/Script/CommanderLlmBotsModule.h`
- `src/Script/CommanderLlmBotsScript.cpp`

## 추가 파일

- `INSTALL_KO.md`
- `deps/LICENSE.nlohmann-json`
- `docs/CHANGES_AUDIT_KO.md`
- `docs/COMPATIBILITY_AUDIT_KO.md`
- `docs/SOURCE_AUDIT.json`
- `docs/UPSTREAM_AUTHORS.md`
- `tools/audit_sources.py`

삭제한 파일은 없습니다. 원본 LlmBots 38개 SHA-256 불변, 로컬 문서 링크 유효, 키/실행 산출물 후보 검사 통과.
실제 Git 포함 후보 전체는 [FILELIST.txt](../FILELIST.txt)를 따릅니다. 소스 감사 JSON은 비교 근거이며 실행 로그가 아닙니다.
