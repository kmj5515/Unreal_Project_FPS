# Resolved Issues

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
