# Unreal_Project_FPS

멀티 FPS(발로란트/오버워치 스타일) 기반 프로젝트.
초기 목표는 "스킬 제외" 상태에서 캐릭터 공통 베이스, 1인칭 시점, 무기 장착/발사(권총/칼) 완성.
GAS(Gameplay Ability System) 기반으로 설계해 이후 스킬 확장을 쉽게 한다.

## 1) 개발 목표 (MVP)

- 멀티플레이 환경에서 2명 이상 정상 접속
- `BaseCharacter` + 자식 2종 구성
- 1인칭 시점 이동/회전/점프
- 공통 무기 시스템(권총, 칼) 장착/전환
- GAS 기반 발사/공격 처리

## 2) 권장 폴더 구조

다음 구조는 C++ 클래스를 기준으로 하며, BP는 각 클래스 파생으로 생성한다.

```text
Source/Ark/
  Core/
    FPSGameMode.h/.cpp
    FPSPlayerController.h/.cpp
    FPSPlayerState.h/.cpp

  Characters/
    BaseFPSCharacter.h/.cpp
    AgentACharacter.h/.cpp
    AgentBCharacter.h/.cpp

  Weapons/
    WeaponBase.h/.cpp
    WeaponPistol.h/.cpp
    WeaponKnife.h/.cpp

  GAS/
    FPSAbilitySystemComponent.h/.cpp
    FPSAttributeSet.h/.cpp
    Abilities/
      GA_WeaponEquip.h/.cpp
      GA_WeaponFirePistol.h/.cpp
      GA_WeaponAttackKnife.h/.cpp
    Effects/
      GE_Damage.h/.cpp (or data-only asset)
      GE_DefaultAttributes.h/.cpp (or data-only asset)

  Input/
    FPSInputConfig.h/.cpp (선택)
```

콘텐츠 폴더 예시:

```text
Content/
  Blueprints/
    Characters/
      BP_BaseFPSCharacter
      BP_AgentACharacter
      BP_AgentBCharacter
    Weapons/
      BP_WeaponBase
      BP_WeaponPistol
      BP_WeaponKnife
  GAS/
    Abilities/
    Effects/
    Tags/
  Animations/
    ABP_FPS_Arms
    Montages/
      Pistol/
      Knife/
  Input/
    IA_Move, IA_Look, IA_Fire, IA_EquipPistol, IA_EquipKnife
    IMC_Default
  Data/
    DA_Weapon_Pistol
    DA_Weapon_Knife
    DT_CharacterDefaults
```

## 3) 클래스 역할 정리

### Core

- `AFPSGameMode`
  - 기본 Pawn/Controller/PlayerState 클래스 설정
- `AFPSPlayerController`
  - Enhanced Input 매핑 등록
  - 입력을 Character 또는 Ability Input으로 전달
- `AFPSPlayerState`
  - `AbilitySystemComponent` 소유(권장)
  - AttributeSet 보유

### Character

- `ABaseFPSCharacter` (공통 부모)
  - 1인칭 카메라/암 메쉬
  - 이동/점프/시점 처리
  - 무기 인벤토리(권총/칼)와 현재 장착 무기 관리
  - 장착/발사 입력 라우팅
  - GAS Avatar 초기화
- `AAgentACharacter`, `AAgentBCharacter` (자식 2종)
  - 외형, 기본 스탯, 애님셋 차이
  - 이후 고유 스킬 Ability 세팅

### Weapon

- `AWeaponBase`
  - 공통 무기 데이터(데미지, 사거리, 발사간격 등)
  - `OnEquip()`, `OnUnequip()`, `CanUse()`
- `AWeaponPistol`
  - 히트스캔 기반 발사
- `AWeaponKnife`
  - 근접 Trace 기반 공격

### GAS

- `UFPSAbilitySystemComponent`
  - Ability 부여/활성화 보조
- `UFPSAttributeSet`
  - `Health`, `MaxHealth`, `MoveSpeed` 등
- `UGA_WeaponEquip`
  - 장착 전환 Ability
- `UGA_WeaponFirePistol`
  - 권총 발사 Ability
- `UGA_WeaponAttackKnife`
  - 칼 공격 Ability
- `GE_Damage`
  - 데미지 적용 Effect
- `GE_DefaultAttributes`
  - 스폰 시 기본 스탯 적용

## 4) 애니메이션 설계 기준 (레이어 권장)

- Base Locomotion(StateMachine): 이동/점프
- Upper Body Slot(몽타주): 발사/근접/장착
- `Layered Blend Per Bone`으로 상체 오버레이
- 권총/칼 각각 장착, 공격 몽타주 분리
- 추후 스킬 몽타주를 같은 슬롯 체계에 추가

## 5) 구현 순서 TODO 체크리스트

### Phase 0 - 프로젝트 준비

- [x] GAS 플러그인 활성화 (`GameplayAbilities`, `GameplayTags`, `GameplayTasks`)
- [x] Enhanced Input 활성화 및 기본 Input Action 생성
- [x] 멀티 접속 테스트 환경 구축(PIE 2인 이상)

### Phase 1 - 코어/캐릭터 골격

- [x] `GameMode`, `PlayerController`, `PlayerState` 생성
- [x] `BaseFPSCharacter` 생성 및 기본 이동/점프/시점 구현
- [ ] 1인칭 카메라 + 팔 메쉬 세팅 (BP에서 Arms Mesh 연결 필요)
- [x] 자식 캐릭터 2종(`AgentA`, `AgentB`) 생성

### Phase 1 진행 메모 (2026-04-07)

- 생성 완료 클래스:
  - `AFPSGameMode`
  - `AFPSPlayerController`
  - `AFPSPlayerState`
  - `ABaseFPSCharacter`
  - `AAgentACharacter`
  - `AAgentBCharacter`
- `DefaultEngine.ini`에 `GlobalDefaultGameMode=/Script/Ark.FPSGameMode` 반영 완료
- 이동 로직은 컨트롤러 Yaw 기준(FPS 표준)으로 반영 완료
- 입력 관련 이슈 해결 기록:
  - 증상: `A/D`는 동작하지만 `W/S` 미동작
  - 원인: 입력 데이터 에셋에서 `IA_Move` Value Type을 `Vector2D`로 설정하지 않음
  - 조치: `IA_Move`를 `Axis2D(Vector2D)`로 수정 후 정상 동작

### Enhanced Input 기준값 (재발 방지)

- `IA_Move`: `Axis2D` 사용
- `IMC_Default` 매핑:
  - `W -> Y +1`
  - `S -> Y -1`
  - `A -> X -1`
  - `D -> X +1`

### Phase 2 - GAS 최소 구성

- [ ] ASC를 `PlayerState`에 생성/소유
- [ ] `AttributeSet` 생성(`Health`, `MaxHealth`, `MoveSpeed`)
- [ ] Possess/Replicate 시점에 `InitAbilityActorInfo` 초기화
- [ ] `GE_DefaultAttributes` 적용

### Phase 3 - 무기 시스템

- [ ] `WeaponBase` 구현(공통 데이터/인터페이스)
- [ ] `WeaponPistol`, `WeaponKnife` 구현
- [ ] Character 인벤토리 슬롯(권총/칼) 구현
- [ ] 장착/해제 로직과 소켓 부착 처리

### Phase 4 - 전투 액션 (GAS)

- [ ] `GA_WeaponEquip` 구현 및 입력 연결
- [ ] `GA_WeaponFirePistol` 구현 (서버 권한 판정)
- [ ] `GA_WeaponAttackKnife` 구현 (근접 Trace)
- [ ] `GE_Damage` 적용 및 피격 체력 감소 확인

### Phase 5 - 애니메이션/동기화

- [ ] ABP에 상/하체 레이어 적용
- [ ] 권총/칼 장착 및 공격 몽타주 연결
- [ ] 원격 클라이언트에서 장착/공격 동기화 확인

### Phase 6 - 데이터 분리/밸런싱

- [ ] 무기 수치 DataAsset/DataTable 분리
- [ ] 캐릭터 기본 수치 DataTable 분리
- [ ] 코드 수정 없이 밸런스 조정 가능한 상태로 정리

## 6) 멀티/GAS 주의사항

- ASC 초기화 타이밍(Possess, OnRep_PlayerState)이 틀리면 Ability 활성화 실패 가능
- 데미지/판정은 서버 권한 우선으로 처리
- 클라이언트는 시각 효과(애니메이션/이펙트) 중심으로 예측 처리
- 상태 태그 예시:
  - `State.Equipping`
  - `State.Attacking`
  - `State.Reloading`

## 7) 다음 작업 후보

- [ ] 죽음/리스폰 파이프라인 (GAS 포함 재초기화)
- [ ] 권총 재장전 시스템
- [ ] 피격 리액션/카메라 흔들림
- [ ] 캐릭터별 고유 Ability(스킬) 1개씩 추가