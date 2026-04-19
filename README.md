# Unreal_Project_FPS

멀티 FPS(발로란트/오버워치 스타일) 프로젝트 메인 문서.
상세 진행 내역은 `docs` 하위 문서로 분리 관리한다.

## 현재 상태

- 현재 단계: **전투 MVP 완료(Phase 3/4)**, **Phase 5(애니·동기화) 진행 중**, **탄약·AC·메뉴/데스매치(1차) 반영**
- 완료 핵심: GAS 데미지 파이프라인, 무기 3슬롯/히트스캔·근접, HUD(체력/탄약) 이벤트 갱신, 로컬 플레이어 전용 크로스헤어 표시, 헤드샷 판정 안정화, 사망 기본 로직, 탄피(`AmmoEject`) 배출
- 추가 반영(코드): **탄창+예비탄(`ReserveAmmo` 복제, `MaxCarryAmmo`·`UWeaponDataAsset`)**, **HUD 탄약 `탄창 / 예비`(예비 없으면 `탄창 / 탄창 최대`)**, **크로스헤어** 무기 텍스처 없을 때 `UFPSCombatComponent` 기본 텍스처(에디터 지정, 엔진 내장 경로 미사용), **`UFPSAttributeSet` Armor/MaxArmor(AC)** 및 피해 시 방어 먼저 소모, **`AFPSDeathmatchGameMode`**(킬 시 `PlayerState` 점수·목표 킬 로그), **메뉴 1차** — `AFPSMainMenuGameMode`, `AFPSMenuPlayerController`, `UFPSMainMenuWidget` / `UFPSLobbyWidget`(에디터에서 WBP·클래스 지정 필요)
- 보류/잔여: 캐릭터 메쉬 이슈 기반 애니 마무리, **무기 장착 애니**, 데스매치 **매치 종료·승리 UI·맵 전환**, **미니맵**, **헤드샷·연속킬(더블~펜타) 등 추가 사운드**, AC **기본 GE·밸런스** 정리

## 빠른 링크

- 개발 로그(날짜별): `docs/DEVLOG/2026-04-07.md`, `docs/DEVLOG/2026-04-08.md`, `docs/DEVLOG/2026-04-09.md`, `docs/DEVLOG/2026-04-10.md`
- 이슈(진행중): `docs/ISSUES/open.md`
- 이슈(해결됨): `docs/ISSUES/resolved.md`
- 무기 추가 가이드: `docs/WEAPONS/add-weapon-guide.md`

## 향후 로드맵 (메타 / UX 플로우)

**나중에 구현 예정.** 지금 Phase(캐릭터·무기·GAS 전투 MVP) 이후에 붙일 목표만 적어 둔다.

### 플레이 흐름

1. **메인 메뉴 맵** — `게임 시작` 등으로 진입
2. **로비 맵** — 본인 캐릭터 표시 + UI 버튼
   - 상점, 뽑기, 옷장, **게임 매칭**
3. **매칭 → 데스매치** — **5킬 선취** 시 매치 종료(규칙은 나중에 조정 가능)
4. **매치 종료 화면** — 보상, **레벨·경험치**, **재화** 반영

### 구현할 때 참고만 (세부는 미정)

- 맵/모드 전환: `OpenLevel`, 서버 `Travel`, `GameInstance`로 로비↔인게임 상태 유지
- 로비 캐릭터: 프리뷰용 Pawn과 실전 매치 Pawn 분리 여부
- 매칭: Listen Server / Dedicated / 추후 온라인 서브시스템 등 선택
- 진행 저장: `SaveGame` 또는 백엔드(계정 연동 시)
- 보상·재화·경험치: `PlayerState`·서브시스템·데이터 테이블 등으로 설계 분리

## 게임플레이 목표 (추가 예정)

문서에만 적어 둔 **설계 방향**이다. 실제 수치·루프는 구현·밸런스 단계에서 조정 가능.

### 무기·탄약·드랍/픽업

- **보유 탄약(목표 수치):** 라이플 **180발**, 스나이퍼 라이플 **30발**, 권총 **48발** (탄창 용량 vs 예비탄 분리는 데이터/무기별로 정리)
- **무기가 처음 생성될 때:** 해당 무기 기준 **풀 탄약**으로 스폰
- **드랍 후 픽업 일관성(멀티):** 클라이언트 1이 장착 중인 무기로 **탄을 모두 소비한 뒤** 버리면, 클라이언트 2가 그 무기를 주웠을 때에도 **탄창·예비 상태가 동일**해야 함 → `AmmoInMagazine`·`ReserveAmmo` 등 **서버 권한 + 복제**로 맞춤(회귀·엣지는 플레이 테스트로 점검)

### HP·AC(방어)

- **HP:** 기존 체력 파이프라인(GAS `Health` / `MaxHealth`)
- **AC:** `UFPSAttributeSet`에 **`Armor` / `MaxArmor`** 추가, 들어오는 `Damage` 처리 시 **방어량만큼 먼저 깎인 뒤** 체력에 반영(에디터에서 `DefaultAttributesEffect` 등으로 초기·최대 방어 수치 지정)

### 미니맵

- 플레이어 위치·기본 방향 등을 보여 주는 **미니맵 UI** 추가(표시 범위·적/아군 표시 규칙은 이후 정리)

## 진행 요약

- [x] 전투 MVP: 라이플/권총/스나이퍼/칼 기본 전투 루프, GAS 데미지/체력 반영
- [x] 무기/입력: 3슬롯 장착 전환, 서버 권한 발사/장전, HUD(체력/탄약) 이벤트 갱신
- [x] FX: MuzzleFlash 정리, `AmmoEject` 기반 탄피(StaticMesh+Impulse+LifeSpan) 적용
- [x] 무기 상호작용: 픽업/드랍(서버 판정 기반), 무기 상태 복제(Equipped/Dropped), 드랍 물리 동기화
- [x] 발사체: `ProjectileBullet` 기반 탄환, 트레이서 파티클, GameplayEffect 기반 피격 처리
- [x] 크로스헤어/탄퍼짐: 사격 누적 퍼짐, 점진 반동(초탄 약/후반 강), 탄퍼짐 연동 튜닝
- [x] 크로스헤어 표시 범위: 무기 장착한 로컬 플레이어 화면에서만 표시되도록 정리
- [x] 디버그 시각화: 히트스캔/발사체 벽·피격 지점 디버그 드로우(멀티캐스트 기반) 추가
- [~] 애니메이션: 기본 ABP/AnimInstance 적용, 몽타주·원격 동기화는 진행 중
- [x] `WeaponBase`: 탄피 배출 임펄스를 `DropImpulseStrength`에서 `ShellImpulseStrength`로 분리 적용
- [x] `WeaponBase`: 탄피가 `Pawn`과 충돌하지 않도록 `ECC_Pawn -> Ignore` 적용
- [x] `BaseFPSCharacter`: 사망 시 캡슐 콜리전 비활성화(시체가 투사체 경로를 막지 않도록)
- [x] `BaseFPSCharacter`: 발사체 채널에서 캡슐 Ignore + 스켈레탈 메시 Block으로 헤드샷 본 판정 안정화
- [x] `FPSAnimInstance`: `bWeaponEquipped` 추가 (AnimBP에서 장착 상태 분기용)
- [x] `FPSCombatComponent`: `HandleFireStarted()`의 빈 로컬 분기 블록 제거
- [x] `FPSCombatComponent`: 미사용 변수 `CrosshairAimFactor` 제거 (실동작 영향 없음)
- [x] `WeaponBase`: 카메라 조준점 기준으로 총구 시작점 탄도 정렬(히트스캔/발사체 공통) 개선
- [x] 킬로그: 서버 킬 이벤트 수집 후 클라이언트 HUD에 `Killer -> Victim (Weapon)` 표시
- [x] 탄약: `WeaponBase` 예비탄 `ReserveAmmo` 복제, `MaxCarryAmmo`·`UWeaponDataAsset::MaxCarryAmmo`, 재장전 시 예비→탄창 보충(라이플 180 / 스나 30 / 권총 48 — 클래스 기본값)
- [x] HUD 탄약: `UFPSHUDWidget`에서 **`탄창 / 예비`**, 예비 없을 때 **`탄창 / 탄창 최대`**
- [x] 크로스헤어: 무기 텍스처 없을 때 `UFPSCombatComponent` **DefaultCrosshair*** 폴백(엔진 리소스 하드코딩 없음)
- [x] GAS AC: `UFPSAttributeSet` `Armor`·`MaxArmor`, 피해 처리 시 방어 우선 소모
- [x] `AFPSGameMode::ReportKill` **virtual**, `AFPSDeathmatchGameMode`(킬 점수·`KillsToWin`)
- [x] 메뉴 C++ 베이스: `AFPSMainMenuGameMode`, `AFPSMenuPlayerController`, `UFPSMainMenuWidget`, `UFPSLobbyWidget`(`OpenLevel` + `listen?game=...FPSDeathmatchGameMode`)

## 다음 작업 (우선순위)

1. **무기별 Idle 애니메이션:** 라이플/권총/스나이퍼 상태별 Idle 분기
2. **피격 피드백:** 히트마커/헤드샷/피격 방향 UI 및 기본 사운드
3. **무기별 퍼짐값 데이터화:** 무기별 `Spread`/`Crosshair` 값 튜닝 테이블 정리
4. **애니·동기화 보강:** 발사/장전/무기 전환의 원격 시각 일관성 점검
5. **피격 판정 보강:** neck/head 본 이름 기반 헤드샷 판정 확장 여부 검토
6. **무기 장착 애니메이션:** 슬롯 전환·픽업 직후 Equip 몽타주(또는 ABP 상태), 필요 시 원격 플레이어 동기화
7. **킬·헤드샷 사운드:** 헤드샷 확정음, 더블킬·멀티킬·쿼드라킬·펜타킬(연속 킬 윈도우·재생 주체는 구현 시 정리)
8. **데스매치 마무리:** 목표 킬 달성 시 **매치 종료·승자 표시·맵/메뉴 복귀** 등 규칙 연결
9. **AC 밸런스:** `DefaultAttributesEffect`·GameplayEffect로 초기 방어·회복·경감 % 등 튜닝
10. **미니맵:** 위젯·월드 미러 등 방식 선택 후 로컬 플레이어 기준 표시
11. **탄약·멀티 QA:** 드랍/픽업·장전 후 HUD·복제 **회귀 테스트**

## 기존 메타 작업 (나중에)

- [~] 게임모드: **`AFPSDeathmatchGameMode`**(킬 점수·목표 킬 로그) — **종료/승리 플로우·UI**는 미구현
- [~] 플로우: C++ **메인/로비 위젯** 제공 — 에디터에서 **WBP·맵·GameMode 오버라이드** 연결, `CreateGame` / `JoinGame` 고도화는 추후
- [ ] 캐릭터 메쉬 이슈 수정: 메쉬 이슈 해결 후 몽타주/원격 애니 동기화 묶음 처리
- [ ] 사운드 추가: 재장전, 칼 공격, **헤드샷·더블/멀티/쿼드/펜타킬**, 무기 장착(Equip) 등 우선 적용

## 나중 작업 메모

- [ ] 언리얼 에디터 기반 `DPS` 측정 툴 제작 (무기별 평균 DPS/유효 사거리/명중률 기반 비교)
- [ ] **사운드:** 무기·이동·피격·UI 등(Phase 5·Weapon 에셋 리스트와 연계, 멀티 재생 규칙은 구현 시 정리)
