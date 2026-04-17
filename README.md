# Unreal_Project_FPS

멀티 FPS(발로란트/오버워치 스타일) 프로젝트 메인 문서.
상세 진행 내역은 `docs` 하위 문서로 분리 관리한다.

## 현재 상태

- 현재 단계: **전투 MVP 완료(Phase 3/4)**, **Phase 5(애니·동기화) 진행 중**
- 완료 핵심: GAS 데미지 파이프라인, 무기 3슬롯/히트스캔·근접, HUD(체력/탄약) 이벤트 갱신, 로컬 플레이어 전용 크로스헤어 표시, 헤드샷 판정 안정화, 사망 기본 로직, 탄피(`AmmoEject`) 배출
- 보류/잔여: 캐릭터 메쉬 이슈 기반 애니 마무리, 게임모드/매치 플로우, 추가 사운드

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

## 다음 작업 (우선순위)

1. **무기별 Idle 애니메이션:** 라이플/권총/스나이퍼 상태별 Idle 분기
2. **피격 피드백:** 히트마커/헤드샷/피격 방향 UI 및 기본 사운드
3. **무기별 퍼짐값 데이터화:** 무기별 `Spread`/`Crosshair` 값 튜닝 테이블 정리
4. **애니·동기화 보강:** 발사/장전/무기 전환의 원격 시각 일관성 점검
5. **피격 판정 보강:** neck/head 본 이름 기반 헤드샷 판정 확장 여부 검토

## 기존 메타 작업 (나중에)

- [ ] 게임모드: 데스매치 GameMode 구현(승리/종료 규칙 포함)
- [ ] 플로우: `GameStartMap -> Lobby` 이동 + `CreateGame` / `JoinGame` 기능 연결
- [ ] 캐릭터 메쉬 이슈 수정: 메쉬 이슈 해결 후 몽타주/원격 애니 동기화 묶음 처리
- [ ] 사운드 추가: 재장전, 칼 공격 사운드 우선 적용

## 나중 작업 메모

- [ ] 언리얼 에디터 기반 `DPS` 측정 툴 제작 (무기별 평균 DPS/유효 사거리/명중률 기반 비교)
- [ ] **사운드:** 무기·이동·피격·UI 등(Phase 5·Weapon 에셋 리스트와 연계, 멀티 재생 규칙은 구현 시 정리)
