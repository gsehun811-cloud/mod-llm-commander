# LLM Commander 한국어 사용자 가이드

> **대상:** AzerothCore WotLK 3.3.5a + 독립 `mod-llm-commander` + 호환 `mod-playerbots`
>
> 본 문서는 저장소의 `conf/mod_llm_commander.conf.dist`, `src/CommanderLlmBotsConfig.cpp`, `src/Script/CommanderLlmBotsScript.cpp`, `src/Script/CommanderLlmBotsModule.cpp`, `src/Bots/CommanderPromptBuilder.cpp`, `data/sql/characters/base/001_llm_commander.sql`을 기준으로 작성했습니다. 설명은 소스의 구현을 나타내며, 모든 빌드/게임 환경에서의 실행 성공을 보장하지 않습니다.

## 빠른 시작

1. **LLM 기능이 내장되지 않은** 호환 Playerbots와 독립 Commander 모듈을 함께 정적 빌드합니다. Playerbots 내부에도 `.llmbot` 명령이 등록된 통합판과는 함께 사용할 수 없습니다. `LlmBots.Enable=0`만으로 중복 등록이 해결되지 않습니다.
2. `configs/modules/mod_llm_commander.conf`를 준비해 LLM을 활성화하고 CM 캐릭터 이름, **사람 계정 ID**, 모델을 지정합니다.
3. Windows에서 API 키를 `LLM_COMMANDER_OPENAI_API_KEY` 환경변수로 등록합니다.
4. 실제 Playerbots 캐릭터 `Cm`을 준비하고 GM 캐릭터로 `.llmbot add Cm adventurer`를 실행합니다.
5. 지정한 사람 계정으로 접속해 `/w Cm 안녕`을 시험하고 `.llmbot stats` 및 서버 로그를 확인합니다.

## 1. CM과 일반 LLM 봇의 차이

- **CM(Commander):** `LlmBots.Commander.Name`에 지정한 기존 Playerbot 캐릭터입니다. `LlmBots.Commander.AccountId`에 지정한 **사람 계정**의 귓속말만 CM 명령 인터페이스로 처리하고, 권한을 다시 검사한 후 허용된 게임 행동 도구를 호출합니다.
- **일반 LLM 봇:** 대화와 게임 상태 조회를 위한 읽기 전용 도구를 사용합니다. 전투·이동·퀘스트의 기본 AI는 기존 Playerbots가 담당합니다. 일반 LLM 봇에 CM 전용 행동 도구가 주어지는 것은 아닙니다.
- **페르소나:** API 요청의 시스템 프롬프트에 포함되는 성격·말투·습관 지침입니다. 실제 Playerbots의 게임 AI나 명령 권한을 변경하지 않습니다.

**주의:** CM을 설정한다고 기존 Playerbots AI의 자율 이동, 전투, 파티 초대 수락이 전부 금지되는 것은 아닙니다. Tool 호출 성공과 게임 행동 완료도 구분해야 합니다.

## 2. 설정 파일

설치된 서버의 설정 파일: `server/configs/modules/mod_llm_commander.conf` (설치 폴더 구조에 따라 `configs/modules/`).

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

- `Commander.Name`: 실제 존재하는 **봇 캐릭터의 이름**. `CM`이 내부적으로 `Cm`으로 정규화될 수 있습니다.
- `Commander.AccountId`: **지휘하는 사람의 계정 ID**. 봇 계정 ID가 **아닙니다**. `0`이면 CM 행동을 거부합니다.
- `ModelFast`, `ModelThink`: 사용할 모델 ID. 위 값은 예시이며 API 계정에서 실제 지원하는 모델로 조정합니다.
- `LlmBots.BotAccount`: 메타데이터이며 봇 계정이나 캐릭터를 자동 생성하지 않습니다.
- `MaxCallsPerMinute`, `MaxTokensPerHour`: 호출/토큰의 **소프트 제한**입니다. 실제 통화 단위 사용료의 하드 한도가 아닙니다.
- `Plan.IntervalSec = 0`: 일반 LLM 봇의 주기적 계획 요청을 끕니다. Commander는 구현상 자율 계획 호출을 별도로 건너뜁니다.

설정은 시작 시 캐시되므로 변경 후 **월드서버 재시작**이 필요합니다. `.llmbot reload`는 roster·페르소나만 재조회합니다.

### 설정 이름 호환성

신규 배포에는 `LlmBots.*` 설정을 권장합니다. 이 독립 모듈은 기존 `LlmCommander.*` 별칭도 읽으며, 별칭에 실제 값이 설정되면 `LlmBots.*`보다 우선합니다. 배포된 `.conf.dist`의 `@legacy@`는 기존 값 상속 표시이므로 임의로 바꾸지 마세요.

## 3. Windows 환경변수로 OpenAI API 키 넣기

**PowerShell**에서 현재 사용자용 영구 환경변수를 설정합니다.

```powershell
[Environment]::SetEnvironmentVariable(
    "LLM_COMMANDER_OPENAI_API_KEY",
    "여기에_본인_OpenAI_API_키",
    "User"
)
```

새 PowerShell에서 값이 등록됐는지만 노출 없이 확인합니다.

```powershell
[bool][Environment]::GetEnvironmentVariable("LLM_COMMANDER_OPENAI_API_KEY", "User")
```

`True`면 등록된 것입니다. 다만 **월드서버를 실행하는 프로세스가 해당 환경변수를 상속해야** 합니다. 이미 실행 중인 월드서버나 오래된 콘솔·런처에서는 적용되지 않을 수 있으므로 실행 프로세스를 새로 시작하세요. 재부팅이 필수는 아닙니다.

일시적인 시험용으로 **현재 PowerShell 세션에만** 설정하려면:

```powershell
$env:LLM_COMMANDER_OPENAI_API_KEY = "여기에_본인_OpenAI_API_키"
```

코드는 `LLM_COMMANDER_OPENAI_API_KEY`와 `LLM_COMMANDER_FOUNDRY_API_KEY` 환경변수를 직접 읽으며 설정 파일 값보다 우선합니다. API 키를 GitHub, 로그, 공개 설정 파일에 넣지 마세요. ChatGPT 구독과 API 과금은 별개입니다.

### HTTPS 인증서 오류

`certificate verify failed`가 발생하면, 신뢰할 수 있는 PEM CA 번들을 준비한 뒤 그 **실제 파일 경로**를 다음 변수로 설정할 수 있습니다.

```powershell
[Environment]::SetEnvironmentVariable(
    "LLM_COMMANDER_CA_FILE",
    "C:\azcorepo\server\ca-bundle.crt",
    "User"
)
```

위 경로는 **예시**입니다. 실제 파일이 존재하는지 먼저 확인하세요. TLS 검증을 끄지 마세요. 자세한 로그와 실제 시험 기록은 [Windows CM 설정 안내](WINDOWS_CM_SETUP_KO.md)를 참조하세요.

## 4. CM 만들기: 사람 계정 ID, 봇 계정 연결, 등록

예시: 사람 계정 `TEST`(ID `101`), 봇 계정 `LLMBOTS`, 기존 봇 캐릭터 `Cm`.

### A. 사람 계정 ID 확인

Characters DB가 아닌 **Auth DB**에서 사람 계정 ID를 조회합니다.

```sql
SELECT id, username FROM acore_auth.account WHERE username = 'TEST';
```

조회한 ID를 `LlmBots.Commander.AccountId`에 넣습니다. 예시 `101`을 실제 값인 것처럼 사용하지 마세요.

### B. 기존 Playerbots 캐릭터 준비

`.llmbot add`는 계정/캐릭터를 만들지 않습니다. 먼저 `Cm`이라는 봇 캐릭터를 Playerbots 계정에 생성해 두어야 합니다. 기존 Playerbots의 절차로 로그인시킵니다.

서로 다른 계정의 봇을 직접 관리해야 하는 환경에서는 다음과 같이 계정 연결이 필요할 수 있습니다.

**봇 계정의 캐릭터로 로그인:**

```text
.playerbots account setKey 별도의보안키
```

**사람 계정의 캐릭터로 로그인:**

```text
.playerbots account link LLMBOTS 별도의보안키
.playerbots account linkedAccounts
.playerbots bot add Cm
```

이 키는 계정 비밀번호나 OpenAI API 키가 아닙니다. 일부 Playerbots 환경에서는 별도 연결이 필요하지 않을 수 있습니다.

### C. GM 게임 캐릭터로 LLM roster 등록

```text
.llmbot add Cm adventurer
.llmbot list
.llmbot stats
```

이 명령은 **월드서버 콘솔용이 아니라 GM 권한이 있는 게임 캐릭터용**입니다. `.llmbot list`의 등록 결과만으로 봇 로그인이나 API 통신 성공까지 보장되지는 않습니다.

CM과 대화:

```text
/w Cm 안녕
/w Cm 현재 파티원 상태를 확인해 줘
```

구현된 CM 도구와 기존 Playerbots의 지원 범위 안에서만 행동을 실행합니다. 실제 동작 전에 소유 계정·세션·봇 권한을 재확인합니다.

## 5. 일반 LLM 봇: 등록과 명령

```text
.llmbot add 철수 scholar
.llmbot add 영희
.llmbot list
.llmbot persona 철수 grump
.llmbot remove 영희
.llmbot reload
.llmbot stats
.llmbot say 철수 안녕
.llmbot goal 철수
.llmbot pause
.llmbot resume
```

- `add 이름 [persona]`: **이미 존재하는 캐릭터**를 등록합니다. 페르소나 생략 시 캐릭터 GUID에 따라 기본 5종 가운데 하나가 선택됩니다.
- `persona 이름 personaID`: 등록된 봇의 페르소나 변경. 없는 ID는 거절합니다.
- `remove`: roster 등록만 삭제하며 캐릭터를 삭제하지 않습니다.
- `reload`: DB의 roster와 페르소나 재조회. 환경변수·모델·일반 설정은 다시 읽지 않습니다.
- `say`, `goal`: 수동 LLM 요청. `pause`/`resume`은 LLM 작업을 중지/재개하며 일반 Playerbots AI 전체를 끄지 않습니다.

## 6. 기본 페르소나 5종

모듈의 `data/sql/characters/base/001_llm_commander.sql`에 정의된 기본 성격입니다.

| 페르소나 ID | 한국어 설명 | 특징 |
| --- | --- | --- |
| `adventurer` | 모험가 | 밝고 호기심이 많으며 새로운 지역과 풍경을 좋아함 |
| `braggart` | 허풍쟁이 | 자신의 영웅담을 과장하고 남의 공도 자랑함 |
| `scholar` | 학자 | 학구적이고 분석적이며 전투 중에도 설명을 늘어놓음 |
| `grump` | 불평쟁이 | 투덜거리지만 속으로는 동료를 아끼는 퇴역 군인 |
| `theorist` | 세계를 의심하는 사람 | 죽음·부활과 반복되는 세계를 이상하게 여김 |

현재 서버에 실제 등록된 종류는 DB에서 조회합니다.

```sql
SELECT persona, display_name, brief, voice, quirks, self_awareness
FROM acore_characters.llmc_bot_persona
ORDER BY persona;
```

## 7. 나만의 한국어 페르소나 등록

독립판은 원본 통합판의 `acore_playerbots.llm_bot_persona`가 아니라 **`acore_characters.llmc_bot_persona`**를 사용합니다.

`.llmbot add 철수 당신은 와우를 즐기는 한국인입니다`처럼 긴 설명을 직접 넣는 방식은 지원하지 않습니다. 먼저 DB에 성격을 만들고 **짧은 persona ID**로 선택합니다.

아래는 **새 ID를 추가**하는 SQL입니다. UTF-8/utf8mb4 연결에서 실행하세요.

```sql
INSERT INTO acore_characters.llmc_bot_persona
(persona, display_name, brief, voice, quirks, self_awareness)
VALUES
(
  'korean_player',
  '한국인 와우 플레이어',
  '당신은 월드 오브 워크래프트를 즐기는 한국인 플레이어입니다. 주변의 동료와 자연스럽게 대화하며 모험을 즐깁니다.',
  '친근하고 자연스러운 한국어 반말로 짧게 대화합니다.',
  '장비와 던전 공략에 관심이 많고 가끔 농담합니다.',
  0
);
```

이 SQL은 같은 `persona` ID가 이미 있다면 중복 키 오류가 발생하므로 기존 항목이 없는지 먼저 확인하세요. 기본 페르소나나 기존 내용을 임의로 덮어쓰지 않는 예시입니다.

게임 내 GM 캐릭터에서:

```text
.llmbot reload
.llmbot add 철수 korean_player
```

이미 철수가 roster에 등록되어 있다면 `add` 대신:

```text
.llmbot persona 철수 korean_player
```

페르소나 열의 의미:

- `persona`: 명령어에서 지정하는 식별자(스키마상 최대 48자)
- `display_name`: 표시 이름
- `brief`: 배경·역할
- `voice`: 말투
- `quirks`: 버릇·특징
- `self_awareness`: 세계의 비현실성을 인식하는 정도. `0`은 게임 세계 안에서 완전히 역할에 몰입하는 설정

코드의 `BuildSystemPrompt`가 `brief`, `voice`, `quirks`, `self_awareness`를 LLM 프롬프트에 포함합니다. 성격은 응답 스타일을 유도하지만 모델이 항상 그대로 답하는 것을 보장하지 않습니다.

## 8. API 사용량 줄이기

선택적인 절약 설정 예시:

```ini
LlmBots.MaxCallsPerMinute = 3
LlmBots.MaxTokensPerHour = 10000
LlmBots.Plan.IntervalSec = 0
```

API 호출 및 토큰 제한은 사용료를 정확히 차단하는 요금 한도가 아닙니다. 이미 진행 중인 요청이 설정값을 넘길 수 있습니다. 일반 봇의 자율 계획은 `Plan.IntervalSec=0`으로 중지할 수 있습니다.

## 9. 문제 해결

| 증상 | 먼저 확인할 것 |
| --- | --- |
| `No such character` | 지정 캐릭터의 실제 존재·이름·캐릭터 캐시 |
| `not my bot` | Playerbots 계정 소유권·링크 설정 |
| `Invalid security key` | 연결 대상 봇 계정에 설정한 보안 키 |
| 등록은 됐는데 로그인하지 않음 | 사람 계정의 월드 접속, Playerbots 로그인 가능 여부 및 watchdog |
| 봇이 응답하지 않음 | `LlmBots.Enable`, API 환경변수 상속, 모델 ID, `.llmbot stats`, `Server.log` |
| `certificate verify failed` | 신뢰 가능한 `ca-bundle.crt`, `LLM_COMMANDER_CA_FILE` |
| 페르소나를 못 찾음 | `acore_characters.llmc_bot_persona`의 ID, `.llmbot reload` |

상세 로그 설정, Windows TLS 문제의 시험 기록은 [Windows CM 실제 설정](WINDOWS_CM_SETUP_KO.md)을 참조하세요.

## 10. 호환성 및 제한

- **통합 LLM Playerbots와 동시 설치 금지:** 양쪽이 `.llmbot`을 등록하므로 중복 충돌합니다.
- **독립판 DB:** `acore_characters.llmc_*`; 원본 통합판 `acore_playerbots.llm_bot_*`에서 자동 마이그레이션되지 않습니다.
- **빌드:** 정적 모듈 구성을 전제로 하며 동적 DLL 연동은 지원하지 않습니다.
- **도구 경계:** 일반 LLM 봇에는 읽기 전용 도구만, CM에는 추가 행동 도구가 부여됩니다. 실제 명령 가능 범위는 Playerbots 구현·권한 검사에 따릅니다.
- **버전 검증:** 저장소의 `0.1.2-audit` 수정본은 소스 감사 상태이며 전체 최신 빌드·실행을 보장하지 않습니다. 과거 Windows 실전 시험과 구분하세요.

관련 문서: [설치 안내](INSTALL_KO.md) · [Windows CM 설정](WINDOWS_CM_SETUP_KO.md) · [호환성 감사](COMPATIBILITY_AUDIT_KO.md).
