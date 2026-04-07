# Unreal_Project_FPS

멀티 FPS(발로란트/오버워치 스타일) 프로젝트 메인 문서.
상세 진행 내역은 `docs` 하위 문서로 분리 관리한다.

## 현재 상태

- 현재 단계: **Phase 1 진행 중**
- 완료: `Phase 0`, `Phase 1` 골격(C++ 클래스/이동 입력)
- 진행 필요: 1인칭 팔 메쉬 BP 연결, `Phase 2(GAS 최소 구성)`

## 빠른 링크

- 개발 로그(날짜별): `docs/DEVLOG/2026-04-07.md`
- 이슈(진행중): `docs/ISSUES/open.md`
- 이슈(해결됨): `docs/ISSUES/resolved.md`

## 체크리스트 (요약)

### Phase 0 - 프로젝트 준비
- [x] GAS 플러그인 활성화 (`GameplayAbilities`, `GameplayTags`, `GameplayTasks`)
- [x] Enhanced Input 활성화 및 기본 Input Action 생성
- [x] 멀티 접속 테스트 환경 구축(PIE 2인 이상)

### Phase 1 - 코어/캐릭터 골격
- [x] `GameMode`, `PlayerController`, `PlayerState` 생성
- [x] `BaseFPSCharacter` 생성 및 기본 이동/점프/시점 구현
- [ ] 1인칭 카메라 + 팔 메쉬 세팅 (BP에서 Arms Mesh 연결 필요)
- [x] 자식 캐릭터 2종(`AgentA`, `AgentB`) 생성

### Phase 2 - GAS 최소 구성
- [ ] ASC를 `PlayerState`에 생성/소유
- [ ] `AttributeSet` 생성(`Health`, `MaxHealth`, `MoveSpeed`)
- [ ] Possess/Replicate 시점에 `InitAbilityActorInfo` 초기화
- [ ] `GE_DefaultAttributes` 적용

### Phase 3~6
- [ ] 무기 시스템
- [ ] 전투 액션(GAS)
- [ ] 애니메이션/동기화
- [ ] 데이터 분리/밸런싱

## 다음 작업 (우선순위)

1. BP에서 `ABaseFPSCharacter` 팔 메쉬 세팅
2. `AFPSPlayerState`에 ASC/AttributeSet 골격 추가
3. `ABaseFPSCharacter`에서 GAS Avatar 초기화 연결