# 한국어 설치 안내

현재 0.1.2-audit는 소스 감사·복원본입니다. 이번 수정본은 컴파일·실행하지 않았습니다.
0.1.1의 과거 빌드 성공을 현재 수정본의 성공으로 해석하지 마십시오.

## 설치와 빌드

AzerothCore 소스의 `modules/mod-playerbots` 옆에 이 모듈을 배치합니다.
LLM 기능이 내장되지 않은 Playerbots가 필요합니다. 내장판도 `.llmbot`을 무조건 등록하므로
`LlmBots.Enable=0`만으로 공존할 수 없습니다. CMake 소스에 이 구조를 거부하는 검사를 추가했습니다.
기존 통합판 파일은 교체·수정하지 마십시오.

사용자가 기존 코어 빌드 절차에서 `MODULES=static`으로 구성하고 다시 빌드합니다.
개별 모듈 옵션도 static/default로 맞춥니다. DLL 방식은 미지원입니다.
복사된 다른 경로의 CMake 캐시는 재사용하지 마십시오. 확인한 커밋은 [호환성 목록](COMPATIBILITY.md)에 있습니다.
VS 2022와 다른 버전은 미검증입니다. 이 감사 작업에서는 CMake·컴파일을 실행하지 않았습니다.

Windows 런타임에는 해당 빌드가 요구하는 x64 OpenSSL·MySQL 클라이언트·VC 런타임이 필요합니다.
이 소스 저장소에 실행 파일, DLL, 테스트 로그, 빌드 결과물은 포함하지 않습니다.

## 스키마

모듈 SQL은 `data/sql/characters/`에 있으며 **Characters DB**용입니다.
`llmc_bot_roster`, `llmc_bot_persona`, `llmc_bot_memory`, `llmc_bot_usage`, `llmc_schema_version`을 사용합니다.
설치 스크립트는 기존 행을 지우지 않는 CREATE IF NOT EXISTS / INSERT IGNORE 방식입니다.

확인한 코어의 `UpdateFetcher::ReceiveIncludedDirectories`는 활성 모듈의 `data/sql/characters`를
검색합니다. 따라서 DB 업데이트 기능이 활성화된 서버를 시작하면 코어가 SQL을 적용할 수 있습니다.
CMake의 install 자체는 SQL 파일 복사만 수행합니다. 소스가 없는 배포 또는 자동 검색을 쓰지 않는 환경에서는
백업 후 제공한 base SQL을 수동 적용하고, 초기 추출본 업그레이드에는 schema_version update를 확인합니다.
이번 감사에서는 서버·DB에 연결하거나 SQL을 실행하지 않았습니다.

원본은 Playerbots DB의 `llm_bot_*`, 독립판은 Characters DB의 `llmc_*`를 사용합니다.
기존 roster/persona/memory를 자동 이전하지 않습니다. 이전이 필요한 경우 별도 백업·명시적 이전 절차가 필요하며
이번 작업에서는 데이터 이전 SQL을 실행하거나 추가하지 않았습니다. 사용량 테이블의 자동 쓰기는 원본에도 없습니다.

## 설정 호환성

업데이트된 `conf/mod_llm_commander.conf.dist`를 설치하고 같은 디렉터리에
`mod_llm_commander.conf`를 생성합니다. 기존 `mod_llm_bots.conf`의 **내용을 그대로** 새 파일명으로 복사할 수 있습니다.
예전 파일명 자체를 자동으로 추가 로딩하지는 않습니다. 코어가 등록한 새 파일명만 사용합니다.

원본 `LlmBots.*` 25개 키를 모두 읽습니다. 이전 독립판의 `LlmCommander.*`도 지원합니다.
템플릿의 `@legacy@`는 기존 키 상속을 의미합니다. 실제 새 키 값이 있으면 해당 항목만 우선하며
0 또는 빈 문자열도 명시한 값으로 취급합니다. 두 이름을 각각 실행하거나 중복 적용하지 않습니다.
기존 독립판 설정을 유지할 때도 새 `.conf.dist`의 별칭 선언을 반드시 함께 설치하십시오.

```ini
LlmBots.Enable = 1
LlmBots.Provider = openai
LlmBots.Commander.Name = Cm
LlmBots.Commander.AccountId = 123
LlmBots.Openai.ModelFast = "사용할 모델 ID"
LlmBots.Openai.ModelThink = "사용할 모델 ID"
```

AccountId는 명령하는 **사람 계정 ID**입니다. 봇 계정 ID가 아니며 0이면 Commander 행동을 거부합니다.
Commander.Name은 실제 존재하는 봇 이름입니다. BotAccount는 원본과 동일한 메타데이터로,
계정 생성이나 자동 로그인 대상 선택에 사용되지 않습니다. 대상은 roster입니다.
Plan.IntervalSec 기본값은 원본의 300초로 복원했으며, 0으로 지정하면 자율 서술 계획을 끕니다.
일반 봇의 전략 변경은 대화 전용 권한 정책에 따라 적용하지 않습니다.

키는 서버 프로세스 환경 변수 `LLM_COMMANDER_OPENAI_API_KEY` 또는 `LLM_COMMANDER_FOUNDRY_API_KEY`로 전달합니다.
이 변수는 두 설정 접두사보다 우선합니다. 기존 ConfigMgr의 AC_* 변수 처리도 그대로 사용합니다.
실제 키·비밀번호·활성 설정 파일은 Git에 넣지 마십시오. Windows OpenSSL에 신뢰 루트가 없으면
신뢰할 수 있는 PEM 번들 경로를 `LLM_COMMANDER_CA_FILE`로 지정합니다. 인증서 검증은 유지합니다.
환경 변수 미지정 시 작업 디렉터리에 기존 `ca-bundle.crt`가 있으면 원본처럼 읽습니다.
설정은 시작 시 캐시되므로 재시작이 필요합니다. `.llmbot reload`는 roster/persona만 다시 읽습니다.

## 등록과 사용

캐릭터와 계정은 Playerbots의 기존 절차로 준비합니다. GM 캐릭터에서 등록합니다.

```text
.llmbot add Cm adventurer
.llmbot add 실제봇이름 scholar
.llmbot list
.llmbot stats
```

원본처럼 지정한 사람 계정이 월드에 들어오면 roster 로그인 작업을 큐에 넣습니다.
10초 watchdog이 오프라인 roster를 재확인하며, 소유자가 로그아웃하면 native 로그아웃 API를 호출합니다.
시작 직후 watchdog에는 원본 60초 지연이 있고, 소유자 로그인 이벤트는 지연을 해제합니다.
접속 성공 여부는 Playerbots의 실제 계정·캐릭터 조건에 따릅니다. 이 복원 동작은 실행 미검증입니다.

사람 소유자 계정에서 Commander에게 귓속말합니다. 일반 봇은 대화 및 읽기 전용 도구만 사용합니다.
이동·정지·전투·추방은 `playerbot_command`를 통해 해당 Playerbots가 지원하는 native 명령과 권한 판정에 따릅니다.
파티 초대는 native join, 레이드 준비는 원본의 소유 Altbot 제한 및 프리셋 사전 검사를 사용합니다.
Tool 전달 성공은 실제 게임 행동 완료와 구분해야 합니다.

`.llmbot pause/resume`은 LLM 요청을 중지/재개하며 Playerbots AI와 소유자 로그인 lifecycle은 유지합니다.
remove는 roster 행만 지우며 캐릭터는 삭제하지 않습니다. 레이드 역할은 세션 메모리입니다.
하위 명령 전체는 README의 명령 표를 확인하십시오.

## 제거 및 사용자 검증

모듈을 disabled로 지정하거나 소스 폴더를 제외한 뒤 코어를 다시 구성·빌드합니다.
기존 DB 테이블은 보존하며 삭제용 SQL은 제공하지 않습니다.
현재 소스는 빌드, 로그인/로그아웃, 명령 실행, 비용 제한, 서버 종료 후 재실행 검증이 남아 있습니다.
검증은 운영 DB와 분리한 테스트 환경에서 수행하십시오.

## 실전 Windows 초기 등록 가이드

계정 연결(Link), `.playerbots bot add Cm`, `.llmbot add Cm adventurer`, OpenAI TLS 인증서 번들 및 오류 로그까지 실제 Windows 시험 서버에서 확인한 절차는 **[Windows CM 초기 등록 및 문제 해결](WINDOWS_CM_SETUP_KO.md)**을 참조하세요. 해당 사례의 계정 ID와 경로는 예시이므로 환경별로 변경해야 합니다.
