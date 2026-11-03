# 문제 해결

| 증상 | 확인할 내용 |
| --- | --- |
| `Embedded LlmBots also registers .llmbot` | 내장 LLM이 없는 Playerbots를 사용합니다. 설정을 끄기만 해서는 명령 충돌을 해결하지 못합니다. |
| `Unsupported Playerbots ...` | 실제 헤더/API가 어댑터와 일치하는지 확인합니다. 강제 캐스팅으로 검사를 우회하지 않습니다. |
| 동적 모듈 거부 | 두 모듈과 MODULES를 static/default로 구성합니다. |
| `.llmbot` 없음 | 모듈 로더 등록, GM RBAC 권한, 게임 내 세션을 확인합니다. 콘솔 명령이 아닙니다. |
| 이전 `.llmcommander` 없음 | 원본 이름 `.llmbot`으로 복원했습니다. |
| 설정이 적용되지 않음 | 새 `.conf.dist`도 설치했는지, 등록된 mod_llm_commander.conf 파일명인지 확인합니다. 실값 LlmCommander 키가 같은 LlmBots 키보다 우선합니다. |
| 기존 LlmBots.Enable로 시작이 거부됨 | 현재 소스에는 예전 Enable 기반 충돌 오검사를 제거했습니다. 오래된 바이너리를 사용 중인지 확인합니다. |
| Commander 응답 없음 | 실제 roster, 이름, 사람 계정 ID, 모델 ID, 환경 키, Playerbots 로그인 조건을 확인합니다. |
| 봇이 자동 로그인하지 않음 | 소유자 계정이 월드 안에 있는지, native QueueOperation/AddPlayerBotOperation API와 봇 계정 조건을 확인합니다. |
| 행동 거부 | 지정 Commander와 현재 온라인 사람 계정만 행동 Tool을 실행합니다. |
| native 명령 실패 | Playerbots 버전이 실제 지원하는 명령/권한/상태 조건인지 확인합니다. |
| 프리셋 없음 | Playerbots의 공개 프리셋 설정에 정확한 이름이 있는지 확인합니다. 레벨 변경 전 거부합니다. |
| schema marker 없음 | Characters DB의 llmc_schema_version과 설치 SQL/코어 모듈 updater 설정을 확인합니다. |
| 옛 roster가 보이지 않음 | 원본 Playerbots DB llm_bot_* 데이터는 자동 이전하지 않습니다. |
| TLS 오류 | 올바른 HTTPS 주소와 신뢰할 PEM 번들을 확인합니다. 검증을 해제하지 않습니다. |
| 제한/일시 오류 | stats와 rate/token/circuit 상태를 확인합니다. 토큰 제한은 통화 단위 지출 한도가 아닙니다. |

현재 소스는 컴파일과 게임 실행 미검증입니다. 제보에는 버전과 비밀을 제거한 오류 정보만 포함하고,
API 키·계정 암호·캐릭터 데이터·운영 로그 전체를 저장소에 올리지 마십시오.

## Windows 설치 중 확인된 실제 오류

`not my bot`, `Invalid security key`, `No such character`, `.llmbot stats`의 errors 증가, `certificate verify failed (SSL routines)` 및 `Logger.root=2`에서 경고 로그가 보이지 않는 문제의 실제 해결 절차는 **[Windows CM 초기 등록 및 문제 해결](WINDOWS_CM_SETUP_KO.md)**에 정리했습니다. 인증서 검증을 해제하지 마세요.
