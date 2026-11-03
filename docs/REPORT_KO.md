> 과거 기록: 이 문서는 **0.1.1 / 2026-11-02** 빌드 시점의 결과입니다.
> 현재 0.1.2-audit의 명령·설정·자동 로그인·SQL 로딩 안내는 [현재 감사 보고서](COMPATIBILITY_AUDIT_KO.md)와
> [설치 안내](INSTALL_KO.md)를 따릅니다. 아래 과거의 수동 SQL 전용/자동 로그인 미지원/내장판 비활성 공존 설명은
> 현재 안내가 아닙니다. 이번 수정본은 컴파일·실행하지 않았습니다.

# 공식 AzerothCore + Playerbots 독립 Commander 검증 결과

공식 Playerbots 단독 전체 빌드와, 호환성을 수정한 독립 Commander 추가 후 전체 worldserver 빌드가
컴파일·링크에 성공했습니다. 게임 내 작동까지 검증한 결과는 아닙니다.

## 1. 실제 소스와 커밋

- 코어: [공식 저장소](https://github.com/mod-playerbots/azerothcore-wotlk), `Playerbot`, `93333d6a9eb00fdf0179adde30da3c17618207d6`
- Playerbots: [공식 저장소](https://github.com/mod-playerbots/mod-playerbots), `master`, `05a1f6fceb8a32aa879b13f5f48934c6e46a774c`
- 기존 독립 모듈 원본: `C:/module/azerothcore-wotlk-Playerbot/modules/mod-llm-commander`
- 검증·수정 복사본: `C:/module/Official-Test/azerothcore-wotlk/modules/mod-llm-commander`
- 빌드/설치: `C:/module/Official-Test/build`, `C:/module/Official-Test/install`

두 공식 저장소는 Git clone으로 받았습니다. 기존 통합판 코어·Playerbots나 CMake 캐시는 사용하지
않았습니다. 공식 Playerbots src/conf에서 LLM·OpenAI·Commander 식별자는 발견되지 않았습니다.
이는 검사한 커밋에 대한 파일/식별자 수준의 확인입니다.

## 2. 빌드 결과

| 검증 | 결과 |
| --- | --- |
| Commander 폴더가 없는 공식 Playerbots 단독 전체 worldserver | 성공, x64 PE |
| 기존 Commander 0.1.0을 그대로 복사한 CMake 구성 | 성공, 소스 21개 및 양쪽 로더 등록 |
| 변경 없는 Commander 결합 컴파일 | 실패: 로그의 `PlayerbotAIConfig.enabled` 참조 |
| 해당 진단 필드 참조 제거 후 전체 결합 빌드 | 성공 |
| 프리셋/SQL 설치 보완 후 최종 전체 worldserver | 성공, x64 PE |
| 오프라인 CTest | 1/1 성공 |
| 설정과 SQL 파일 설치 | 설정 1개, SQL 2개 내용 일치 |

사용자 승인에 따라 설치된 Visual Studio Community 2026, MSVC 19.51, Ninja, x64 Release,
정적 modules/scripts로 검증했습니다. VS 2022는 미설치로 미검증입니다. CMake 4.3.1-msvc1,
Boost 1.92, OpenSSL 3.6.4, MySQL 8.4 개발 라이브러리를 사용했으며 도구를 설치하지 않았습니다.
논리 CPU 4개/RAM 15.75 GiB에서 병렬도 2로 시작해 결합 빌드는 여유 RAM을 확인하며 3으로 제한했습니다.
코어/스크립트 PCH 옵션을 활성화했습니다. 공식 modules CMake의 모듈 PCH 코드는 주석 처리되어 있습니다.
Debug 컴파일러 검사에 `/Z7`을 지정했으며 Release 최적화는 유지했습니다.

## 3. 원본 보존과 신규 모듈 변경

기존 개발 프로젝트에 쓰기 작업을 하지 않았으며, 모듈 원본 64개 파일의 SHA-256을 다시 확인했습니다.
공식 코어와 Playerbots는 최종 Git 추적 파일 변경 0개이고 HEAD SHA도 그대로입니다.
Playerbots 엔진은 새 모듈에 복사하지 않았습니다.

다음 구현 변경은 **Official-Test 안의 Commander 복사본만** 대상으로 했습니다.

- `src/Bots/CommanderLlmBotBrain.cpp`: 호환되지 않는 enabled 필드의 진단 로그 참조 제거. AI 동작은 변경하지 않음.
- `src/Compat/CommanderPresetPolicy.h`: native 특성 프리셋의 이름·빈 슬롯 규칙을 검사하는 독립 정책 추가.
- `src/Compat/CommanderPlayerbotsAdapter.h`: 공식 공개 프리셋 인터페이스 검사와 어댑터 선언 추가.
- `src/Compat/CommanderPlayerbotsAdapter.cpp`: 공식 공개 설정 배열을 어댑터에서 조회.
- `src/Bots/CommanderLlmToolbox.cpp`: 레이드 준비 시 레벨 변경 전에 누락 프리셋 거부.
- `tests/commander_offline_tests.cpp`: 프리셋 누락·대소문자·빈 이름·비활성 슬롯 회귀 검사 추가.
- `mod-llm-commander.cmake`: 필수 설정 헤더 검사와 SQL 파일 설치 규칙 추가. SQL 실행 없음.
- `VERSION`: 공식 소스 검증 보완판을 0.1.1-experimental로 구분.

공식 Config에는 대문자 `Enabled`가 있지만, 이름에 다시 종속되지 않도록 진단에 필요 없는 값만
제거했습니다. 레이드 준비에서는 설정에 실제로 존재하는 프리셋인지 확인한 뒤 레벨을 바꿉니다.
native 액션의 반환 true는 게임 행동 완료 보장이 아니므로 런타임 확인이 별도로 필요합니다.
문서, 버전, 파일 목록·해시도 이 검증 결과에 맞게 갱신했습니다.

## 4. 설치·의존성

`conf/mod_llm_commander.conf.dist`가 독립 모듈 설정으로 등록되고, 격리된 설치 경로의
`configs/modules`에 원본과 같은 내용으로 설치되는 것을 확인했습니다.
초기 버전은 SQL 파일을 설치하지 않아 CMake 설치 규칙을 보완했습니다. 최종 설치 경로는
`sql/mod-llm-commander/characters/base/001_llm_commander.sql` 및
`characters/updates/001_schema_version.sql`입니다. 파일만 설치하며 DB 연결/SQL 실행은 하지 않습니다.
Characters DB의 모듈 전용 `llmc_*` 테이블을 수동 설치해야 합니다.

정적 링크가 성공했고 dumpbin으로 직접 DLL 의존성을 검사했습니다. 추가 직접 의존성:
libssl-3-x64.dll.
실제 프로세스 시작과 DLL 검색 경로/인증서 로딩은 미검증입니다. 동적 모듈은 명시적으로 지원하지 않습니다.
기본 설정은 비활성이고 AccountId=0은 행동 권한을 거부합니다. 실제 설정/키는 소스에 넣지 않습니다.

## 5. 통합판과 기능 차이 및 코드 점검

| 기능 | 보존 방식과 검증 범위 |
| --- | --- |
| OpenAI·자연어 명령 | Responses provider와 구조화 응답/Tool 루프 포함, 컴파일 및 스키마 단위 검사; 실제 API 미호출 |
| Commander·사용자 권한 | 실행 시 Commander GUID·AI, 온라인 사람, 허용 계정 재확인; 허용/거부 오프라인 검사 |
| 봇 검색·파티 초대 | 공식 랜덤봇 관리자와 native join 사용, 헤더/액션 생성자 확인; 실제 초대 미검증 |
| 추방·이동·정지·전투 | 현재 사람이 master인 대상에게 native 명령 전달; 각 명령의 실제 행동 미검증 |
| 주변 플레이어·상황 조회 | 기존 읽기 Tool 보존, 컴파일 확인 |
| 레이드 | 세션 역할 저장, 소유 Altbot 레벨/특성/장비 준비; 32 프리셋 기본값 비교 및 누락 프리셋 거부 테스트 |
| 일반 LLM 봇 | 대화·읽기 전용 Tool만 제공; 행동 Tool은 공통 실행 경계에서 Commander에게만 허용 |
| 요청·비용 제한 | 큐/호출률/토큰 gate 단위 검사; 토큰 한도는 soft limit이며 달러 비용 hard cap 아님 |
| 오류 처리 | HTTPS 검증/시간 제한, 대기 요청 세대 무효화, 권한 기본 거부; 실제 네트워크 장애 미검증 |

독립 이름공간·파일명·로더·명령 `.llmbot`·설정 `LlmCommander.*`·테이블 `llmc_*`를 사용합니다.
기존 통합판의 `LlmBots.*` DB/설정 데이터를 자동 이전하지 않습니다. 중복 legacy brain이 활성화되면
시작을 거부합니다. 일반 봇의 LLM agenda가 전투/이동 전략을 변경하지 않게 했습니다.
전투·퀘스트·자동 이동 AI와 봇 로그인은 기존 Playerbots가 담당합니다.
Foundry는 대화만 지원하며 행동 Tool provider는 OpenAI입니다. Mock은 고정 대화입니다.

## 6. 미구현·미검증 항목

- 서버 실행, 로그인, DB 스키마 적용/업데이트, 실제 OpenAI/Foundry 요청과 게임 내 모든 행동.
- VS 2022, 다른 공식 커밋 조합, Linux, 동적 DLL 모듈, 패키지만 새 PC에서 재설치하는 검사.
- 실제 DLL/인증서 로딩, 파티 초대 완료와 레이드 준비 결과·장비 상태.
- 달러 단위 hard cap, `llmc_bot_usage` 영속 집계, LLM 기반 장기 기억 자동 기록.
- 모듈 물리적 제거 후 최종 전체 재빌드는 반복하지 않았습니다. 대신 처음부터 모듈 없는 단독 전체 빌드는 성공했습니다.

이 작업에서는 서버·서비스를 실행하지 않았고 DB/API 연결, 운영 경로 수정, GitHub push/PR을 하지 않았습니다.

## 7. 다른 버전 확장

이 커밋 조합의 정적 컴파일만 검증한 것으로, 모든 Playerbots 버전 호환을 뜻하지 않습니다.
CMake는 실제 헤더와 AI 조회 인터페이스를 탐지하고 `src/Compat`가 native 호출을 격리합니다.
새 버전은 실제 헤더에 맞는 어댑터와 인터페이스 검사를 추가한 뒤, LLM이 없는 해당 소스 쌍에서
단독/결합 전체 빌드와 권한·명령 실행을 검증해 지원 표에 추가해야 합니다.
내부 메모리 조작·강제 캐스팅·SHA 하드코딩으로 검사를 우회하지 않습니다.

## 8. 재현 자료

`C:/module/Official-Test/validation`의 configure/build 로그, `official-sources.json`,
`baseline-result.json`, `combined-final-result.json`, `module-compatibility-fixes.json`,
`module-fixes.json`, `install-final.json`, `test-offline.log`, dependencies 로그를 확인하십시오.
원래 실패한 `build-combined-as-is.log`도 보존했습니다. 순수 Playerbots 실행 파일과 최종 실행 파일은
`C:/module/Official-Test/results`에 각각 보관했습니다. 실행은 하지 않았습니다.
