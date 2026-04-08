# Resolved Issues

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
