# Acore-Integrated Windows 실전 설정 및 오류 해결

> 2026-11-04 독립 mod-llm-commander + mod-playerbots 테스트. 기존 Acore-Portable 원본은 수정하지 마세요. 완전 포터블 배포·최종 OpenAI 자연어 응답·기존 DB 이관은 아직 별도 검증이 필요합니다.

## 먼저 기억할 7가지

1. **LLM-SETUP.bat는 서버 시작 BAT가 아닙니다.** OpenAI 키 설정용이며 Enter는 기존 설정을 유지하고 종료하는 정상 동작입니다. 실제 통합 install 폴더에서 START/STOP BAT는 발견되지 않았습니다.
2. LlmBots.Enable=1만으로는 불충분합니다. ModelFast가 빈 값이면 LLM bots disabled 오류가 발생합니다.
3. Commander.AccountId는 LLMBOTS가 아니라 **지휘하는 사람 TEST의 숫자 계정 ID**입니다.
4. `.playerbots bot add Cm`은 월드 로그인, `.llmbot add Cm adventurer`는 LLM roster 등록입니다.
5. OpenAI API 키, WoW 계정 비밀번호, 게임 안 `.playerbots account setKey`의 보안키는 서로 다릅니다.
6. 목록에 서버 이름이 보여도 실제 월드 접속은 Realm 주소·플래그와 8085 포트에 따라 실패할 수 있습니다.
7. 게임 서버의 legacy.dll/libcrypto/libssl을 동일한 호환 버전으로 맞추되 MySQL 전용 OpenSSL DLL은 따로 둡니다.

## 1. 폴더·시작 순서

실제 테스트 폴더: `C:\module\Acore-Integrated\install` (worldserver.exe, authserver.exe, dbimport.exe, mysql, configs, LLM-SETUP.bat).

시작 순서: **MySQL → authserver → worldserver**. 기존 운영 DB와 3306/3724/8085 포트가 겹치지 않는지 먼저 확인합니다. 동일 mysql/data를 대상으로 mysqld를 중복 실행하지 마세요. 3306은 로컬 전용으로 유지합니다.

```powershell
# MySQL 실행 중인지 먼저 확인하고, 별도 창에서 authserver를 실행
Set-Location 'C:\module\Acore-Integrated\install'
.\authserver.exe -c '.\configs\authserver.conf'
# worldserver는 또 다른 PowerShell 창에서 실행
Set-Location 'C:\module\Acore-Integrated\install'
.\worldserver.exe -c '.\configs\worldserver.conf'
```

포트는 `Get-NetTCPConnection -State Listen -LocalPort 3306,3724,8085`로 점검합니다. Windows Private 방화벽에서 게임용 TCP 3724/8085만 필요에 따라 허용합니다.

**dbimport.exe**: 이번 실제 테스트에서는 `Applied 730 queries. Containing 1287 new and 1384 archived updates.` 다음 `Halting process...`, 종료 코드 `0`이었습니다. 이는 임포트 성공이며, 원래 계정·캐릭터·봇 DB 이전 완료를 의미하지 않습니다. 이미 적용한 업데이트를 반복하지 마세요.

## 2. DLL 시작 지점 오류 — 실제 해결

| 파일 | 잘못된 install | 빌드 폴더 |
|---|---|---|
| legacy.dll | 3.6.4 | 3.6.4 |
| libcrypto-3-x64.dll | **3.5.7** | 3.6.4 |
| libssl-3-x64.dll | **3.5.7** | 3.6.4 |
| libmysql.dll | 8.4.11 | 8.4.11 |

**해결:** 서버를 정상 종료하고 기존 두 OpenSSL DLL을 백업한 뒤, 같은 빌드의 x64-Release 설치 산출물에서 libcrypto/libssl **3.6.4 두 파일만** 통합 install에 복사하고 해시 확인. 서버가 정상 실행됐습니다. `install\mysql\bin`의 OpenSSL 3.5.7은 **변경하지 않았습니다**.

## 3. 목록에는 Realm이 보이지만 접속 안 됨

다른 클라이언트 PC에서 접속할 때 `acore_auth.realmlist.address`가 `127.0.0.1`이면 문제가 됩니다. **클라이언트 IP가 아닌 서버 PC LAN IP**를 사용하세요. 실제 N150 서버 IP는 `192.168.0.102`였습니다.

```powershell
$mysql='C:\module\Acore-Integrated\install\mysql\bin\mysql.exe'
& $mysql -h 127.0.0.1 -P 3306 -u root -p -D acore_auth -e 'SELECT id,name,address,localAddress,port,flag FROM realmlist;'
```

이번에는 주소를 서버 LAN IP로 수정했으며, 뒤이어 발견한 `flag=1`을 `flag=0`으로 변경하자 접속이 성공했습니다. `flag=0`은 일반 Realm 온라인 표시값입니다. 서버 프로세스가 꺼져 있으면 플래그만 바꿔 해결되지 않습니다. SQL 변경 전 **지금 3306 포트를 소유한 MySQL의 실행 경로/데이터 폴더가 통합 서버인지 확인하고 백업**하세요.

```sql
-- IP는 본인의 서버 LAN IP로 교체 후 실행
UPDATE realmlist SET address='192.168.0.102',localAddress='192.168.0.102',flag=0 WHERE id=1;
```

클라이언트 `realmlist.wtf`의 `set realmlist 192.168.0.102`도 서버 IP를 가리켜야 합니다.

## 4. CM 설정 — 이 값이 빠지면 disabled

설정 파일: `install\configs\modules\mod_llm_commander.conf`

```ini
LlmBots.Enable = 1
LlmBots.Provider = openai
LlmBots.Commander.Name = Cm
LlmBots.Commander.AccountId = 101
LlmBots.BotAccount = llmbots
LlmBots.Openai.ModelFast = "gpt-4.1-mini"
LlmBots.Openai.ModelThink = "gpt-4.1-mini"
```

**101은 이 테스트의 TEST 사람 계정 ID 예시**입니다. `SELECT id,username FROM acore_auth.account WHERE username='TEST';`로 확인하고 각자 값을 사용하세요. 테스트에서는 처음 Enable=0, ModelFast/Think 공백, Commander.AccountId=0이었습니다. 로그에 `Provider 'openai' configuration failed: LlmBots.Openai.ModelFast is not configured - LLM bots disabled`가 실제로 찍혔습니다. 모델과 사람 계정 ID를 변경하면 **worldserver 재시작** 필요. `LlmCommander.* = @legacy@`는 설정 호환 별칭입니다.

OpenAI API 키는 Windows 환경변수 `AC_LLM_BOTS_OPENAI_API_KEY`에서 감지됐으며 별도 재발급이 필요 없었습니다. `LLM_COMMANDER_OPENAI_API_KEY`도 지원합니다. LLM-SETUP.bat는 키 설정 전용이며 모델/계정 ID를 자동 설정하지 않습니다. 민감한 키는 어떤 로그에도 공개하지 마세요.

## 5. 게임 안에서 CM 연결

**모든 다음 명령은 WoW 게임 채팅창에서 실행**합니다.

LLMBOTS 계정의 CM 캐릭터로 접속:

```text
.playerbots account setKey <직접_정한_연결용_보안키>
```

TEST 계정으로 전환:

```text
.playerbots account link LLMBOTS <같은_보안키>
.playerbots account linkedAccounts
.playerbots bot add Cm
.llmbot add Cm adventurer
.llmbot list
/w Cm 안녕
.llmbot stats
```

`.llmbot`은 GM 권한 필요. 고권한 GM 부여는 개인 테스트에 한정하고 배포본에서는 최소 권한 정책을 고려하세요. `.llmbot list` 성공은 **API 호출 성공이 아니며**, 이미 등록한 CM은 중복 추가하지 마세요.

## 6. Hm. 응답 및 TLS CA 인증서

`Hm.`은 OpenAI 호출 실패 시 폴백일 수 있습니다. API 키가 없다는 의미로 단정하지 마세요. 게임의 `.llmbot stats` 및 `install\Server.log`에서 `LLM call failed`, `certificate verify failed`, `HTTP 401/400/429`를 확인합니다. **별도 Windows 테스트에서** TLS 인증서 검증 오류가 CA 번들로 해결됐으며, 이번 통합 서버에서도 CA 번들이 없어 준비했습니다. 단 이번 서버의 실제 TLS 오류 로그는 아직 확인하지 않았습니다.

CA 출처: https://curl.se/docs/caextract.html / https://curl.se/ca/cacert.pem

```powershell
$install='C:\module\Acore-Integrated\install'
$ca=Join-Path $install 'ca-bundle.crt'
if(!(Test-Path $ca)) { Invoke-WebRequest 'https://curl.se/ca/cacert.pem' -OutFile $ca }
# 기존 worldserver 종료 후, 월드서버를 실제 시작하는 동일 PowerShell 창에서:
$env:LLM_COMMANDER_CA_FILE=$ca
Set-Location $install
.\worldserver.exe -c '.\configs\worldserver.conf'
```

`$env:`는 현재 PowerShell 창과 자식 프로세스에만 적용됩니다. 추후 자동 START BAT에는 worldserver 실행 직전 CA 환경변수 설정이 필요합니다. TLS 검증을 끄지 마세요.

## 7. 아직 끝나지 않은 작업

- 새 통합 서버에서 자연스러운 OpenAI 응답(폴백 `Hm.` 제외) 최종 확인
- 서버 시작·종료 START/STOP BAT 제작·검증
- 소스 폴더 및 개발 도구가 없는 다른 Windows PC에서 완전 포터블 실행 검증
- 기존 운영 서버 계정·캐릭터·봇 DB 이전(현재 임포트만 완료)
- 신규 Playerbot 한글 이름 생성 기능(기존 봇은 검증 전 삭제 금지)
- 시대 모듈은 사용자 결정대로 현재 변경하지 않음

함께 볼 문서: [Windows CM 설정](WINDOWS_CM_SETUP_KO.md), [설치](INSTALL_KO.md), [호환성](COMPATIBILITY.md).