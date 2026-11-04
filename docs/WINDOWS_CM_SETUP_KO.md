# Windows 실제 설치: CM 계정 연결·등록·API 오류 해결

> **Acore-Integrated 통합 서버 실전 가이드:** [Windows 설치·DLL·Realm·LLM 문제해결](ACORE_INTEGRATED_WINDOWS_KO.md) — 2026-11-04 실제 테스트에서 발견한 오류와 해결 순서를 정리했습니다. 완전 포터블 배포 및 최종 API 응답은 별도 검증이 필요합니다.


> 2026-11-03 사용자 테스트에서 독립 모듈의 빌드, CM 등록, 로그인, OpenAI 응답을 확인했습니다. 이 결과는 전체 배포 환경의 회귀 테스트가 아닙니다. 기존 0.1.2-audit 작성 시점의 '빌드/실행 미검증'과 구분합니다.

## 1. 계정 및 설정

예시: 사람 계정 `TEST` (ID 101), 봇 계정 `LLMBOTS` (ID 102), 봇 캐릭터 `Cm`. **ID는 실제 서버별로 확인하세요.** `CM`이라는 이름이 `Cm`으로 정규화될 수 있습니다.

`configs/modules/mod_llm_commander.conf` 설정 예:

```ini
LlmBots.Enable = 1
LlmBots.Provider = openai
LlmBots.Commander.Name = Cm
LlmBots.Commander.AccountId = 101
LlmBots.Openai.ModelFast = "gpt-4.1-mini"
LlmBots.Openai.ModelThink = "gpt-4.1-mini"
```

`Commander.AccountId`는 **사람 계정 ID**이지 봇 계정 ID가 아닙니다. `LlmBots.BotAccount`는 자동 계정 생성/로그인 설정이 아닙니다. API 키는 공개 설정/저장소 대신 서버 프로세스 환경변수에 전달합니다.

## 2. Playerbots 계정 연결 (다른 계정의 봇 직접 관리 시)

`.llmbot add`는 계정 생성도 로그인도 하지 않습니다. 직접 관리에 계정 연결이 필요한 경우:

1. **LLMBOTS** 계정의 게임 캐릭터로 로그인하여 보안 키 설정:

   ```text
   .playerbots account setKey 별도의비밀보안키
   ```

2. **TEST** 계정의 게임 캐릭터로 로그인하여 봇 계정 연결 및 확인:

   ```text
   .playerbots account link LLMBOTS 별도의비밀보안키
   .playerbots account linkedAccounts
   ```

3. TEST 캐릭터에서 기존 CM을 Playerbots로 로그인:

   ```text
   .playerbots bot add Cm
   ```

`not my bot`은 Playerbots 소유·계정 연결 문제일 수 있습니다. `Invalid security key`는 **연결 대상 계정에 등록된 키**와 입력 키가 일치하는지 확인하세요. 봇 계정의 보안 키와 계정 암호/API 키는 서로 다른 것입니다. 환경에 따라 계정 연결이 필요하지 않을 수 있습니다.

## 3. LLM roster 등록 (GM 게임 내 명령)

기존 `Cm` 캐릭터를 생성·확인한 뒤 **GM 게임 캐릭터**에서 실행합니다.

```text
.llmbot add Cm adventurer
.llmbot list
.llmbot stats
```

`.llmbot list`에 `Cm`이 나오면 roster 등록 성공이지만 **게임 접속 또는 API 성공을 의미하지 않습니다.** `No such character`는 이름/캐릭터 존재/서버 캐시를 확인하세요. 사용자 테스트에서는 수동 Playerbots 접속 후 roster 등록에 성공했습니다. 원본 자동 로그인 watchdog은 구현되어 있지만 이 사례만으로 모든 환경의 자동 로그인 성공을 보장하지는 않습니다.

## 4. Windows TLS 인증서 오류 해결

API 키가 환경변수에서 발견돼도 실제 HTTPS 호출은 실패할 수 있습니다. 실제 오류:

```text
LLM call failed (bot_utterance): certificate verify failed (SSL routines)
```

출처가 신뢰되는 최신 PEM CA 번들 `ca-bundle.crt`를 준비하고, **월드 서버를 실행하는 동일 PowerShell**에서 설정합니다.

```powershell
$install = "C:\module\azerothcore-wotlk\out\install\x64-Release"
$env:LLM_COMMANDER_CA_FILE = "$install\ca-bundle.crt"
Set-Location $install
.\worldserver.exe -c ".\configs\worldserver.conf"
```

배치 파일이 `worldserver.exe`와 같은 폴더에 있다면 서버 실행 직전:

```bat
set "LLM_COMMANDER_CA_FILE=%~dp0ca-bundle.crt"
```

`$env:` 설정은 현재 PowerShell 세션에만 유효합니다. **인증서 검증은 끄지 마세요.** 이 모듈에는 현재 `.conf`용 `LlmBots.Openai.CaFile` 키가 구현돼 있지 않으므로 임의 추가해도 효과가 없습니다.

## 5. API 실패 로그와 토큰 비용

`.llmbot stats`에서 `errors`가 증가하고 CM이 `Hm.` 같은 고정 응답만 하면 API 실패를 의심하세요. `worldserver.conf`의 `Logger.root=2`는 `LOG_WARN`을 숨길 수 있습니다. 상세 확인용으로 **중복 없이** 추가하고 재시작:

```ini
Logger.llmcommander=5,Console Server
```

테스트:

```text
/w Cm 안녕
.llmbot stats
```

`Server.log`에서 `LLM call failed`와 HTTP/TLS 사유를 확인하세요. 인증서 경로를 지정한 후 실제 테스트에서는 API 응답에 성공했습니다.

```ini
LlmBots.Plan.IntervalSec = 0
LlmBots.MaxCallsPerMinute = 3
LlmBots.MaxTokensPerHour = 10000
```

이 값들은 선택적인 비용 절약 예시입니다. CM은 코드에서 이미 자율 LLM 계획을 건너뛰지만, `Plan.IntervalSec=0`은 **일반 roster 봇의 자율 계획도 중단**합니다. 호출/토큰 제한은 **엄격한 달러 한도가 아니며** 진행 중 호출이 초과할 수 있습니다.

## 6. CM 동작의 경계

독립 LLM brain은 CM의 일반 say/yell에 반응하지 않고, 지정 사람 계정의 **귓속말**만 명령 인터페이스로 처리하도록 구현되었습니다. 그러나 일반 Playerbots AI가 하는 **자율 이동·전투·파티 초대 수락을 자동으로 완전히 금지하는 기능은 아닙니다.** 실제 시험에서 CM의 파티 초대가 가능했습니다. 비전투/무적/초대 거부를 보장하는 것으로 설명하지 마세요.
