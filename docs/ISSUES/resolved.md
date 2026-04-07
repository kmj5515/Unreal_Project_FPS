# Resolved Issues

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
