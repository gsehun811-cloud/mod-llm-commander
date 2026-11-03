# 독립 LLM Commander 소스 감사 및 배포 준비

감사일: 2026-11-03 / 소스 버전: 0.1.2-audit

## 범위와 비교 근거

수정 범위는 이 독립 모듈 디렉터리뿐입니다. 원본 통합 프로젝트, 공식 코어/Playerbots, Portable-Test,
운영 서버·DB는 수정하지 않았습니다. CMake 구성, 컴파일, 서버 실행, 실제 SQL 실행, API 호출은 하지 않았습니다.

비교 기준은 사용자가 지정한 로컬 `mod-playerbots/src/LlmBots`의 C++/헤더 **38개 전부**입니다.
각 파일의 SHA-256과 독립판 대응 파일, 이름 정규화 후 차이 수를 [SOURCE_AUDIT.json](SOURCE_AUDIT.json)에 기록했습니다.
이 값은 기능 보존율이 아니라 재검토용 소스 지문입니다. 파일 이름·namespace·헤더 가드 차이도 차이에 포함됩니다.

참조 GitHub HEAD는 조회 시
[45ac63a5b7a4182df22873f20b7834ca3afb3b9e](https://github.com/gsehun811-cloud/mod-llm-playerbots-commander/commit/45ac63a5b7a4182df22873f20b7834ca3afb3b9e)였습니다.
Git tree blob과 로컬 파일을 LF로 정규화해 비교했습니다. 26개는 동일했고, 11개는 배포/변경 고지 차이,
OpenAiProvider.cpp 1개는 고지와 요청 Tool 개수·prepare_raid_character 유무의 진단 로그 차이였습니다.
Brain/Module의 특별 고지와 Provider의 실제 diff를 직접 확인했습니다. 로컬판의 기능적 확장 차이로
오인하지 않았으며 누락된 upstream 고지를 독립판에 복구했습니다. LICENSE는 참조의 Git blob과 일치합니다.
저작권자 목록은 UPSTREAM_AUTHORS.md, nlohmann MIT 전문은 deps/LICENSE.nlohmann-json에 보존했습니다.

## 보존율을 판단할 수 있는 범위

- 원본 파일 대응: **38/38**. Playerbots 엔진 소스는 복사하지 않았습니다.
- 명령 등록 및 핸들러 소스: **10/10 (100%)**. 이름 정규화 후 등록 권한·인수·핸들러 본문을 대조했습니다.
- Tool 선언: **16/16 (100%)**. 이름·설명·JSON 인수 정의와 역할별 배정이 원본과 같습니다.
- 기존 설정 키 및 코드 기본값: **25/25 (100%)**. 양쪽 읽기 선언을 대조했습니다.

이 수치는 각각 선언/핸들러 비교의 일치율입니다. **전체 기능 보존율이나 실행 성공률 100%를 뜻하지 않습니다.**
아래 정책 차이와 미검증 항목 때문에 전체 기능을 단일 백분율로 보증하지 않습니다.
소스 비교는 tools/audit_sources.py로 재현할 수 있습니다. 원본 경로는 인수로 전달하며 읽기만 합니다.

## 명령어 복원

등록 루트를 `.llmcommander`에서 원본 `.llmbot`으로 복원했습니다. 이전 이름의 별칭은 추가하지 않았습니다.
모든 하위 명령은 원본과 같은 `rbac::RBAC_PERM_COMMAND_GM`, `Console::No`입니다.
모듈 namespace/loader/config 파일명은 충돌 분리를 위해 독립 이름을 유지합니다.

| 하위 명령 | 원본과 동일한 인수 | 동작 및 확인 범위 |
| --- | --- | --- |
| list | 없음 | enabled roster 나열. disabled 항목은 원본 Entries(false)에서도 제외됨 |
| add | name [persona] | 기존 캐릭터만 등록, 기본 성격 guid 해시 선택, 최대 등록 수 검사 |
| remove | name | roster 행만 제거, 캐릭터 삭제 없음 |
| persona | name persona | 존재하는 성격으로 변경 |
| reload | 없음 | roster/persona만 조회, config 재로딩 아님 |
| stats | 없음 | 메모리 집계 표시, DB 영속 집계 아님 |
| say | name prompt... | ForceSocial 호출, 원본 인수 결합/전달 보존 |
| goal | name | ForcePlan 호출. 전략 적용 차이는 아래 별도 기록 |
| pause | 없음 | 일시 중지. 기존 독립판의 budget/generation 보호 유지 |
| resume | 없음 | 재개. 이전 요청의 늦은 행동 실행 방지 유지 |

명령 표·핸들러 일치와 별개로 backend의 DB 위치/권한 강화/goal 전략 억제는 아래 차이가 있습니다.

## 기능별 구현 비교

| 기능 | 원본 구현 근거 | 현재 결과 |
| --- | --- | --- |
| Commander 등록/식별 | CmdAdd, Commander.Name, SubmitSocial | 기존 캐릭터 roster 등록 보존. 실행 권한용 이름 정규화 유지. 전용 캐릭터 생성 명령은 원본에도 없음 |
| 자동 로그인 | OnPlayerLogin, EnsureBotsSpawned | 삭제된 owner hook·watchdog 복원. 소유자 접속 시 지연 해제, 재확인은 10초, 시작 지연은 60초 |
| 자동 로그아웃 | OnPlayerLogout | 삭제된 native LogoutPlayerBot 호출 복원. 모듈 Shutdown 자체가 모든 봇을 로그아웃시키는 것은 원본에도 없음 |
| 소유자 인증 | CommanderAccountOnline, OnWhisperToRosterBot | 원본 계정 ID 검사 보존. AccountId=0이면 Commander 요청/행동 거부 |
| 대화/자연어 처리 | SubmitSocial, PromptBuilder, ApplyUtterance | 프롬프트/구조화 응답/귓속말 전달 보존. 실행 시점만 world thread로 분리 |
| OpenAI | OpenAiProvider::Complete | 원본 Responses payload·Tool/transcript·사용량 파싱 보존. 로컬 원본의 진단 로그도 보존 |
| Foundry/Mock | FoundryProvider, MockProvider | 원본과 동일한 대화 경로. Foundry 행동 Tool과 Mock 행동 시뮬레이션은 원래 없음 |
| 봇 검색 | ToolFindPlayerbot | 같은 맵/진영의 온라인 random bot 검색·직업/레벨/거리 필터 보존 |
| 파티 초대 | join_playerbot_to_group | 원본 native join 동작 보존. 실제 파티 합류 완료와 Tool 수락은 별개 |
| 추방/이동/정지/전투 | playerbot_command | 새 독립판의 추가 GetMaster==issuer 제한 제거, 원본처럼 native ParseChatCommand의 권한·명령 판정으로 위임. 별도 kick Tool은 원본에도 없음 |
| 소유자 순간이동 | teleport_owner_to_playerbot/city | 원본 대상/도시 및 상태 검사 보존 |
| 레이드 역할 | set_raid_roles/list_raid_roles | 원본 세션 메모리 저장·조회 보존. DB 영속 기능은 원본에도 없음 |
| 레이드 준비 | prepare_raid_character | 원본 역할/특성/레벨/장비 준비 흐름 및 소유 Altbot 제한 보존. 기존 독립판의 native 프리셋 존재 사전 검사는 유지 |
| 일반 파티원 LLM | SocialToolSpecs, 일반 brain | 원본 8개 읽기 Tool과 대화 보존. 일반/파티 채팅 자동 청취 훅은 원본에도 연결되지 않았고 귓속말 위주. OnSayOrYellNear 함수만으로 자동 청취가 된다고 보증하지 않음 |
| Commander Tool 권한 | CommanderToolSpecs, ExecuteTool | 원본 8개 Commander 선언 복원. 일반 봇은 읽기 전용. 실행 시 Commander GUID/AI·온라인 사람 계정을 재검사하는 독립판 보호는 유지 |
| 계획/전략 | SubmitPlan, ApplyAgenda | 기본 주기 0→300초 복원. 일반 봇의 strategy_toggles → ChangeStrategy만 Commander 이름 검사로 차단. CM의 수동 goal 및 기존 행동 Tool은 유지 |
| 비용/요청 제한 | LlmBudget, LlmClient | 원본 분당/시간당/queue/80% Think 제한 보존. 후속 Tool round와 재시도도 제한, 소수 토큰 refill/queue 경합 수정 유지 |
| 오류 처리 | provider/HTTPS/fallback/circuit | 원본 fallback/retry/circuit 보존. 잘못된 Tool JSON 거부, worker 예외 처리, deadline·응답 크기 제한 유지 |
| DB 저장/조회 | ReloadRoster, ApplyMemoryForGuid, Cmd* | roster/persona CRUD·memory 조회 보존. 독립 Characters DB llmc_* 사용. 기존 데이터를 자동 공유/이전하지 않음 |
| 장기 기억/사용량 | memory 조회, usage 테이블 | 원본에도 자동 memory 생성/usage INSERT 없음. 신규 구현으로 보고하지 않음 |
| Chronicler/대화 guard | ChroniclerBot, LlmConversation | 원본 구현 보존. 시간값은 현재 코어 epoch 초 반환에 맞춘 독립판 수정 유지 |

## 복원한 불필요 변경

1. 명령 루트 및 관련 설치/설정 안내의 `.llmbot` 표기.
2. 사람 소유자 로그인/로그아웃 이벤트, roster 자동 로그인, 10초 watchdog과 초기 지연.
3. Plan.IntervalSec 기본 300초와 BotAccount 기본 llmbots, 누락된 템플릿 옵션.
4. 원본에 없던 설정값 강제 제한(worker 1~8, roster 1~100, timeout 1~120초) 제거.
5. DB에서 읽은 기존 roster의 brain 수를 MaxRosterSize로 잘라내던 독립판 처리 제거.
6. playerbot_command의 추가 master 일치 제한 제거. native 권한 판정은 그대로 유지.
7. Commander에게 SocialToolSpecs까지 합쳐 광고하던 확장 제거. 원본 역할별 8+8 선언 복원.
8. HTTPS의 작업 디렉터리 ca-bundle.crt 로딩 복원. 환경 변수 지정이 우선하며, 파일이 없으면 기본 신뢰 경로 사용.
9. 참조 GitHub 배포·저작권 고지 복원 및 제3자 라이선스 별도 보존.

## 설정 호환 계층

원본 LlmBots.* 25개를 기본으로 읽고 이전 독립판 LlmCommander.*도 유지합니다.
새 키가 @legacy@이면 기존 키 상속, 실제 값이면 새 키 우선입니다. 0/빈 문자열은 명시적 override입니다.
API-key 전용 환경 변수가 최종 우선하며 설정은 한 번만 적용합니다. 구형 활성 설정의 내용은 새 등록 파일명
mod_llm_commander.conf로 복사할 수 있습니다. 기존 파일명 자동 로딩은 추가하지 않았습니다.

원래 이름을 LlmCommander로 바꾼 이유는 통합판과의 이름 분리였지만, 단독 공식 환경에서 기존 옵션을
못 쓰게 만드는 것은 불필요했습니다. 이제 양쪽을 지원합니다. 단, 통합판은 비활성이라도 `.llmbot`을
등록하므로 **동시 설치는 미지원**이며 CMake 소스에서 명확히 거부합니다. 단순 Enable 검사로 별칭 설정을
통합판 활성화로 오인하던 초기화 검사는 제거했습니다. 실제 CMake 검사는 사용자의 다음 구성 시 실행됩니다.

## 유지한 차이와 남은 호환성

- DB가 Playerbots DB llm_bot_*에서 Characters DB llmc_*로 분리됨. 스키마 marker가 없으면 시작 중단.
  별도 명시적 데이터 이전이 필요합니다. 자동 이전/원본 테이블 공유는 구현하지 않았습니다.
- 원본 LoadLlmBotsConfig는 호출마다 ConfigMgr를 읽지만 독립판은 시작 시 캐시를 유지합니다.
  코어 config reload만으로 즉시 반영되지 않으며 설정 변경 뒤 재시작해야 합니다.
- 원본의 map-thread brain tick을 world-thread로 옮겨 Tool/roster 변경과 실행 스레드를 맞춤.
  맵 이벤트는 잠금된 event bus를 사용하고 원자 enabled/paused 플래그를 유지합니다.
  일부 이벤트의 roster 확인도 map-thread에서 brain map을 직접 조회하는 대신 registry 조회를 유지합니다.
- 일반 봇의 자율·수동 계획 응답은 전략을 변경하지 않고 목표/서술만 반영합니다. CM의 수동 goal은 원본 전략 토글을 유지합니다. CM 행동 Tool 경로는 변경하지 않았습니다.
- 명령 Tool은 실행 경계에서 소유자와 Commander를 다시 확인함. 잘못된 arguments는 기본값 진행 대신 오류 반환.
- pause/reload 뒤 오래된 completion 차단, follow-up/retry 제한 및 circuit/queue 경합 보호는 유지함.
  원본보다 후속 요청이 일찍 거부될 수 있습니다. pause 시 새 brain의 생성/상태 등 모든 경계 상황은 실행 미검증입니다.
- URL은 HTTPS만 허용하고 경로 끝 슬래시를 정리함. 원본의 http:// 파싱 수용과 다르며 실제 평문 transport는 원본에도 없음.
  TLS 인증·deadline·2 MiB 제한 및 환경키 지원 유지. 작업 디렉터리 CA 호환을 복원했지만 Windows 신뢰 경로는 실행 확인 필요.
- 정적 모듈만 지원. 외부 공개 Playerbots API 의존성은 그대로이며 다른 버전은 어댑터/API 검증이 필요함.
- 원본의 불완전한 부분도 명시함: 자동 일반/파티 채팅 청취, 사용량 영속화, 자동 장기 기억 생성,
  Foundry 행동 Tool, 통화 단위 hard cap, 레이드 역할 영속 저장은 구현 완료로 보증하지 않음.

## 검증과 배포 상태

소스 대조와 C++ 스타일 검사를 수행했습니다. 원본 LlmBots 38개는 작업 전후 SHA-256으로 불변 여부를 확인합니다.
SQL 수정은 명령 이름 주석뿐이며 실행문 변경은 없습니다. 코어 SQL linter 전체 실행은 git fetch와
모듈 외부 저장소 변경을 유발하므로 수행하지 않았고, 로컬 검사 함수만 불러 제한적으로 확인했습니다.
SQL 로컬 검사 중 INSERT/DELETE 선행 검사 7곳은 기존 INSERT IGNORE 방식 때문에 실패했습니다.
다른 실행한 검사(공백/문법 형태/세미콜론/backtick/InnoDB)는 통과했습니다. 전체 SQL lint 통과로 보고하지 않습니다.
독립 모듈의 기존 비파괴 INSERT IGNORE seed 방식은 코어의 DELETE 선행 규칙과 다르며 기존 데이터를 지우지 않는
사용자 요구에 따라 유지합니다. SQL이 새로 실행되거나 DB가 변경된 것은 아닙니다.

0.1.1의 과거 공식 정적 빌드 결과와 0.1.2-audit의 소스 검사 결과는 구분했습니다.
현재 수정본의 컴파일, 연결, 자동 로그인/로그아웃, 실제 Tool·OpenAI 요청, 게임 내 권한·정지/재개는 미검증입니다.
핵심 재검증 대상은 복원한 world-thread 로그인 큐 API와 소유자 로그아웃, 별칭 설정 우선순위,
업데이트된 .conf.dist의 설치, native 명령 권한, 프리셋 검사, 오래된 요청 거부입니다.
CMake 구성/빌드는 사용자가 별도로 진행해야 합니다.

README, LICENSE, AUTHORS, INSTALL_KO, 호환성·문제 해결 문서와 소스/설정/SQL/오프라인 테스트 소스를 정리했습니다.
FILELIST.txt는 배포 후보 파일 전체, SHA256SUMS는 그 중 자기 자신을 제외한 해시입니다.
실제 키·활성 설정·DB 데이터·캐릭터 데이터·로그·실행 파일·빌드 결과는 포함하지 않습니다.
추가 제외 규칙을 .gitignore에 기록했습니다. Git 초기화, 저장소 생성, Push/PR은 하지 않았습니다.
GitHub 공개는 사용자의 최종 승인 후 별도 작업입니다. 변경 파일 목록은 CHANGES_AUDIT_KO.md를 확인하십시오.
