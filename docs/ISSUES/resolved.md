# Resolved Issues

## 2026-04-11 - 멀티에서 피격자 본인 화면에 사망 몽타주가 안 보임
- 증상: 클라 A가 클라 B를 죽일 때 A 화면(또는 B의 원격 표현)에서는 사망 몽타주가 보이나, **B 본인 클라 화면**에서는 안 보임
- 원인: 1P용으로 **전신 `Mesh`**에 **`Owner No See`**(또는 숨김)를 쓰면, `Multicast_OnDeath`에서 `GetMesh()`에 몽타주를 재생해도 **소유 클라에서는 메쉬가 렌더되지 않음**
- 해결: `Multicast_OnDeath`에서 **`IsLocallyControlled()`**이면 `Mesh`에 `SetOwnerNoSee(false)`, `SetVisibility(true, true)` 적용(필요 시 **Capsule**도 `SetVisibility`). 리스폰 시 새 폰이면 BP 기본값으로 다시 숨김 처리됨; 동일 폰 부활 시에는 가시성·`OwnerNoSee`를 다시 맞출 것

## 2026-04-11 - 사망 몽타주 종료 후 Idle로 돌아감
- 증상: 사망 몽타주가 끝나면 슬롯이 비워져 기본 로코모션(Idle)이 다시 보임
- 원인: 몽타주 기본 동작이 블렌드 아웃 후 슬롯 가중치를 0으로 되돌리기 때문
- 해결: **에디터**에서 사망 몽타주 에셋의 **Enable Auto Blend Out** 끄기, 또는 AnimBP에서 `bDead` 등으로 사망 포즈 레이어 유지(프로젝트는 에디터 설정으로 처리)

## 2026-04-10 - 앉기(Crouch) 입력은 되는데 동작 안 함
- 증상: `Crouch()` 호출 시 `CanEverCrouch=0`, `LogCharacter: ... crouching is disabled on this character! (check CharacterMovement NavAgentSettings)`
- 원인: 캐릭터 BP의 **Character Movement → Nav Agent**에서 **`Can Crouch`**가 꺼져 있어 `CanEverCrouch()`가 false. C++ 생성자에서 켠 값이 BP 컴포넌트 기본값에 덮임
- 해결:
  - `ABaseFPSCharacter::BeginPlay`에서 `GetCharacterMovement()->NavAgentProps.bCanCrouch = true` 재설정
  - 에디터에서도 동일 BP에 **Nav Agent → Can Crouch** 켜 두면 재발 방지에 유리

## 2026-04-09 - 모듈러 의상 스켈레탈 메쉬가 본체와 따로 노는 문제
- 증상: 캐릭터 본체 애니메이션은 정상인데, 본체 `Mesh`의 자식인 의상 스켈레탈 메쉬들이 동작을 제대로 따라가지 않음
- 원인: 의상 메쉬가 본체 메쉬 포즈를 리더-팔로워 구조로 복사하도록 설정되지 않음
- 해결:
  - 캐릭터 BP의 Construction Script에서 각 의상 SkeletalMeshComponent에 `Set Leader Pose Component` 적용
  - Leader는 본체 `Mesh`로 지정하여 의상 메쉬가 본체 애니메이션 포즈를 동일하게 따르도록 설정

## 2026-04-09 - 히트스캔 `Miss` 과다 발생
- 증상: 상대를 조준해도 `Miss`가 자주 발생
- 원인: 히트스캔이 `ECC_Visibility` 단일 채널 기준이라 캐릭터 충돌 설정에 따라 플레이어를 놓침
- 해결:
  - `ECC_Visibility` + `ECC_Pawn` 동시 라인트레이스
  - 둘 다 히트 시 더 가까운 충돌점 채택
  - 디버그 시각화(`bDebugDrawTrace`)로 라인/충돌점 확인 가능하도록 추가

## 2026-04-09 - `Data.Damage` SetByCaller magnitude 미설정 에러
- 증상: `FGameplayEffectSpec::GetMagnitude ... Data.Damage ... had not yet been set by caller`
- 원인: `GE_Damage`와 무기 측 SetByCaller Tag 세팅 불일치(또는 누락)
- 해결:
  - 코드에서 강제 보정 대신 누락 시 명확한 에러 로그로 진단 가능하게 유지
  - 에디터에서 `GE_Damage` Modifier의 SetByCaller Data Tag와 무기 `DamageSetByCallerTag`를 `Data.Damage`로 통일

## 2026-04-09 - 시작 체력 `1`로 보이는 문제
- 증상: `GE_DefaultAttributes`에서 `Health=100` 세팅했는데 시작 시 체력이 `1`
- 원인: 기본 `MaxHealth=1` 상태에서 `Health`가 먼저 적용되어 clamp로 `1`이 됨(적용 순서 영향)
- 해결:
  - `UFPSAttributeSet::PostGameplayEffectExecute`에서 `MaxHealth` 갱신 시 초기 구간(`Health <= 1`)은 `Health = MaxHealth`로 보정

## 2026-04-08 - Phase 1 전신 메쉬 + 카메라(BP)
- BP `Mesh`에 전신 스켈레탈 지정, `head`(또는 `CameraAttachSocketName`) 기준 카메라 부착 확인

## 2026-04-08 - GE_DefaultAttributes (CurveTable) 적용
- 방식: CurveTable → `GE_DefaultAttributes` → 서버에서 `TryApplyDefaultAttributes()`로 1회 적용
- 검증: 동작 확인 완료

## 2026-04-07 - GAS Attribute 확인
- 검증: 콘솔 `ShowDebug AbilitySystem`
- 결과: `Health`, `MaxHealth`, `MoveSpeed` 속성 표시 및 반영 확인
- 상태: 이슈 없음

## 2026-04-07 - `W/S` 입력 미동작
- 증상: `A/D`는 동작하지만 `W/S` 미동작
- 원인: `IA_Move` 데이터 에셋의 Value Type이 `Axis2D(Vector2D)`가 아니었음
- 해결:
  - `IA_Move` Value Type을 `Axis2D(Vector2D)`로 설정
  - `IMC_Default`에서 `W/S`를 Y축으로 매핑
- 재발 방지:
  - 기준값: `W=Y+1`, `S=Y-1`, `A=X-1`, `D=X+1`
