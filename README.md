# Unreal_Project_FPS (Ark)

**UE5 기반 1인칭 멀티 FPS** 프로젝트입니다. `Ark` 런타임 모듈에서 **Gameplay Ability System(GAS)**, **Enhanced Input**, **리슨 서버 복제**를 전제로 한 전투·무기·HUD 파이프라인을 C++ 중심으로 구성합니다.

---

## 포트폴리오 요약

- **전투 루프:** 무기 3슬롯, 히트스캔·발사체·근접, 서버 권한 발사·장전, 탄창/예비탄 복제 및 HUD 연동  
- **데미지:** GAS 기반 속성·GameplayEffect 경로, 헤드샷 등 판정 보강  
- **개발 생산성:** 플레이 중 **F10** 인게임 디버그 패널(`UFPSDebugToolWidget`)로 트레이스·DPS·퍼짐·무한 탄약 등 튜닝·QA 지원  
- **문서화:** 이슈는 해결 후 **[해결된 이슈](docs/ISSUES/resolved.md)**에 증상·원인·해결·검증을 남기고, 진행 중 항목은 **[진행 중 이슈](docs/ISSUES/open.md)**에서 관리합니다. (논의·디버깅 과정도 동일 형식으로 이슈 문서에 옮겨 둠.)

상세 아키텍처·디렉터리 맵·기술 선택 근거는 **[Ark 기술 문서 (한국어)](docs/portfolio/Ark-Technical-Document-KO.md)**를 참고하세요.

---

## 문서 링크

| 문서 | 설명 |
|------|------|
| [docs/ISSUES/open.md](docs/ISSUES/open.md) | 진행 중 이슈 |
| [docs/ISSUES/resolved.md](docs/ISSUES/resolved.md) | 해결 완료 이슈(증상·원인·해결·검증, 논의 요지 정리) |
| [docs/portfolio/Ark-Technical-Document-KO.md](docs/portfolio/Ark-Technical-Document-KO.md) | 포트폴리오용 기술 문서 |
| [docs/WEAPONS/add-weapon-guide.md](docs/WEAPONS/add-weapon-guide.md) | 무기 추가 가이드 |
| [docs/DEVLOG/](docs/DEVLOG/) | 일자별 개발 로그 |

---

## 저장소 구성

- **`Ark/`** — UE 프로젝트(`Ark.uproject`) 및 `Source/Ark/` C++ 모듈  
- **`docs/`** — 이슈, 무기 가이드, 포트폴리오 문서, DEVLOG  

---

무기 데이터·에셋 파이프라인은 [무기 추가 가이드](docs/WEAPONS/add-weapon-guide.md)를 따릅니다.
