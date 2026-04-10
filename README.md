# Unreal_Project_FPS

멀티 FPS(발로란트/오버워치 스타일) 프로젝트 메인 문서.
상세 진행 내역은 `docs` 하위 문서로 분리 관리한다.

## 현재 상태

- 현재 단계: **Phase 3·4 MVP 수준 + 사망 기본 로직**, **Phase 5(애니·동기화) 진행 중**
- 완료: 코어/캐릭터, 전신 메쉬+카메라(BP), GAS·기본 스탯·데미지 파이프라인, 무기 3슬롯·히트스캔/근접, Enhanced Input(이동·앉기 등), `FPSAnimInstance` 기본 변수·`bDead`, HUD 위젯(체력/탄약 텍스트) 이벤트 기반 갱신
- **사망(기본):** `Health` ≤ 0 → 서버에서 `State.Dead`, 어빌리티 취소·이동/입력 차단, **Reliable `Multicast_OnDeath`**로 `DeathMontage` 재생
- **HUD/탄약 동기화(정리):** 다중 클라에서 간헐적으로 Ammo `0/0`이 뜨던 문제는 **무기 이벤트 의존**을 줄이고, 캐릭터 복제 필드(`HUDAmmoInMag`, `HUDMagSize`) + `OnRep` 재브로드캐스트로 안정화
- **남은 MVP 성격 작업:** 리스폰·매치 규칙 등 **사망 이후** 흐름, 원격 애니/상태 동기화(Phase 5). **사운드는 MVP 이후** 단계적으로 추가 예정

## 빠른 링크

- 개발 로그(날짜별): `docs/DEVLOG/2026-04-07.md`, `docs/DEVLOG/2026-04-08.md`, `docs/DEVLOG/2026-04-09.md`, `docs/DEVLOG/2026-04-10.md`
- 이슈(진행중): `docs/ISSUES/open.md`
- 이슈(해결됨): `docs/ISSUES/resolved.md`

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

## 체크리스트 (요약)

### Phase 0 - 프로젝트 준비
- [x] GAS 플러그인 활성화 (`GameplayAbilities`, `GameplayTags`, `GameplayTasks`)
- [x] Enhanced Input 활성화 및 기본 Input Action 생성
- [x] 멀티 접속 테스트 환경 구축(PIE 2인 이상)

### Phase 1 - 코어/캐릭터 골격
- [x] `GameMode`, `PlayerController`, `PlayerState` 생성
- [x] `BaseFPSCharacter` 생성 및 기본 이동/점프/시점 구현
- [x] **전신 메쉬** + 카메라: BP `Mesh`에 스켈레탈 지정, `head`(또는 눈 위치) 소켓 있으면 카메라 자동 부착 (없으면 캡슬 오프셋 유지)
- [x] 자식 캐릭터 2종(`AgentA`, `AgentB`) 생성
- [x] 앉기: `IA_Crouch` → `CrouchAction`, `Started`/`Completed`로 `Crouch`/`UnCrouch` (hold). **BP Character Movement → Nav Agent → `Can Crouch`** 켜거나, 코드 `BeginPlay`에서 `NavAgentProps.bCanCrouch` 보강
- [x] `FPSAnimInstance`: `Speed`/`Direction`, `bCrouching`, `bJumping`, `bJumpPressed`, `bEnableJump`
- [x] 모듈러 의상 스켈레탈 메쉬 동기화: BP Construction Script에서 `Set Leader Pose Component`로 본체 `Mesh`를 Leader로 지정
- [x] `FPSPlayerController` 로컬 HUD 위젯 생성/`AddToViewport` + Pawn 재소유(`OnPossess`/`OnRep_Pawn`) 시 재바인딩

### MVP 마무리 (이후 Phase 5와 연결)
- [x] **사망(기본):** `Health` ≤ 0 → `State.Dead`·입력/이동·발사 차단·`Multicast_OnDeath` 몽타주·로컬 메쉬 가시성(피격자 본인 화면). **몽타주 끝난 뒤 Idle 복귀 방지** 등은 에디터(예: 몽타주 **Enable Auto Blend Out** 끄기)에서 조정
- [ ] **사망 이후:** 리스폰·관전·라운드 종료 등 게임 규칙(미구현)
- [ ] **애니·동기화:** 원격 플레이어 무기·로코모션·앉기 등이 보이도록 복제/ABP 정리

### Phase 2 - GAS 최소 구성
- [x] ASC를 `PlayerState`에 생성/소유
- [x] `AttributeSet` 생성(`Health`, `MaxHealth`, `MoveSpeed`)
- [x] Possess/Replicate 시점에 `InitAbilityActorInfo` 초기화
- [x] `GE_DefaultAttributes` 적용 (CurveTable 연동 GE, 서버 1회 적용)

### Phase 3 - 무기 시스템
- [x] `WeaponBase` (공통: 장착/해제, 소켓 부착, 발사 인터페이스)
- [x] 무기 파생 클래스 생성: `RifleWeapon`, `PistolWeapon`, `KnifeWeapon`
- [x] 캐릭터 3슬롯: `Primary(라이플)`, `Secondary(권총)`, `Melee(칼)` + `CurrentWeapon` 전환/복제
- [x] 입력 라우팅 준비: `FireAction`, `EquipPrimaryAction`, `EquipSecondaryAction`, `EquipMeleeAction`
- [x] 서버 권한 전환/발사: `ServerEquipWeapon`, `ServerSetFiring`
- [x] 에디터 세팅: 캐릭터 BP에서 `PrimaryWeaponClass/SecondaryWeaponClass/MeleeWeaponClass` 할당
- [x] 소켓: `WeaponAttachSocketName` 기본값은 `WeaponSocket` (스켈레톤/메쉬 소켓 이름과 일치 필요)
- [x] 실제 전투 MVP 구현(서버 권한): 라이플/권총=히트스캔(LineTrace), 칼=근접 Sweep(Sphere)
- [x] 데미지 적용 GAS 경로 추가: 무기에서 `GE_Damage` 우선 적용 (없으면 UE PointDamage fallback)
- [x] 에디터에서 `GE_Damage` 에셋 생성/연결 (`Data.Damage` SetByCaller 사용)
- [x] 코드 기반 데이터 분리: `WeaponDataAsset` + Native Tag(`Data.Damage`) 도입
- [x] 에디터에서 `DA_Weapon_Rifle/Pistol/Knife` 생성 후 각 무기 BP에 `WeaponData` 할당
- [x] Trace 디버그 시각화 옵션 추가 (`bDebugDrawTrace`, `DebugDrawDuration`)
- [x] 히트스캔 판정 보강: `Visibility + Pawn` 동시 트레이스 후 가까운 히트 선택

### Phase 4 - 전투 액션 (GAS)
- [x] `GA_WeaponEquip` C++ 스캐폴딩
- [x] `GA_WeaponFireRifle` C++ 스캐폴딩
- [x] `GA_WeaponFirePistol` C++ 스캐폴딩 (1회 발사 형태)
- [x] `GA_WeaponAttackKnife` C++ 스캐폴딩 (근접 1회 발사 형태)
- [x] `GA_WeaponReload` C++ 스캐폴딩 + `IA_Reload` 입력 경로 연결
- [x] `GE_Damage` 수용 코드: `Damage` Attribute → `Health` 반영 파이프라인 구현
- [x] GameplayTag(`State.Attacking`, `State.Reloading`) NativeTag 추가 + Ability 충돌 방지 기본 적용
- [x] HUD 동기화: 체력/탄약 텍스트를 Tick 없이 이벤트 기반(`OnHUDHealthChanged`, `OnHUDAmmoChanged`)으로 갱신
- [x] 탄약 복제 보강: `AmmoInMagazine`/`OwnerCharacter` `OnRep` + 캐릭터 HUD 전용 복제 필드(`HUDAmmoInMag`, `HUDMagSize`)로 초기값(`00`) 및 다중 클라 `0/0` 누락 없이 HUD 갱신
- [ ] 피격 피드백(선택): 카메라/사운드는 클라, 데미지는 서버

### Phase 5 - 애니메이션 / 동기화
- [ ] ABP: 하체 로코모션 + 상체 레이어(또는 슬롯) 구조
- [~] 권총: Fire / Reload / Equip 몽타주 연결 (`Reload`는 코드 멀티캐스트/태그 연결 완료, ABP 슬롯·가시성 검증 남음)
- [ ] 칼: Equip / Melee 몽타주 연결
- [ ] Ability 타이밍과 몽타주(Notify) 정렬
- [ ] 멀티: 원격 플레이어 무기·상태가 보이도록 애님/복제 점검 (복제 기본은 완료, 애님 시각화 검증 남음)
- [~] **사운드:** 무기 발사 사운드(`FireSound`)는 발사 FX 멀티캐스트에 연동 완료. 재장전/빈 탄/근접·캐릭터·UI는 추가 예정
- [x] MuzzleFlash·히트 이펙트 스폰 위치 정리
- [x] 탄피 배출(StaticMesh): `AmmoEjectSocketName`(`AmmoEject`) 소켓 기준 스폰 + 물리 Impulse + 수명(`ShellLifeSpan`) + 무기별 `WeaponDataAsset` 분리

### Phase 6 - 데이터 분리 / 밸런싱
- [x] 무기별 `DataAsset` 또는 `DataTable` (현재: `WeaponDataAsset` 적용)

## 다음 작업 (우선순위)

1. **Phase 5(묶음):** 원격 플레이어 **애니메이션·무기·상태** 동기화 + Reload 몽타주 시각화 안정화  
   (캐릭터 메쉬/몽타주 이슈 정리 후 한 번에 처리)
2. **HUD 회귀 테스트(미검증):** 체력/탄약 텍스트의 초기화·무기 교체·다중 클라(3인+) 시점 검증
3. **사운드(부분 완료):** `FireSound` 이후 재장전/빈 탄/근접 + 캐릭터/환경/UI 확장
4. **사망 이후(보류):** 리스폰·관전·UI는 GameMode 규칙 확정 후 구현

## 나중 작업 메모

- [ ] 언리얼 에디터 기반 `DPS` 측정 툴 제작 (무기별 평균 DPS/유효 사거리/명중률 기반 비교)
- [ ] **사운드:** 무기·이동·피격·UI 등(Phase 5·Weapon 에셋 리스트와 연계, 멀티 재생 규칙은 구현 시 정리)
