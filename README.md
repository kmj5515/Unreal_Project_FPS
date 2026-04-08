# Unreal_Project_FPS

멀티 FPS(발로란트/오버워치 스타일) 프로젝트 메인 문서.
상세 진행 내역은 `docs` 하위 문서로 분리 관리한다.

## 현재 상태

- 현재 단계: **Phase 2 진행 중**
- 완료: `Phase 0`, `Phase 1` 골격, GAS 골격(ASC/AttributeSet/이속 연동) 대부분
- 진행 필요: 1인칭 팔 메쉬 BP 연결, `GE_DefaultAttributes` 적용

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
- [x] ASC를 `PlayerState`에 생성/소유
- [x] `AttributeSet` 생성(`Health`, `MaxHealth`, `MoveSpeed`)
- [x] Possess/Replicate 시점에 `InitAbilityActorInfo` 초기화
- [ ] `GE_DefaultAttributes` 적용

### Phase 3 - 무기 시스템
- [ ] `WeaponBase` (공통: 장착/해제, 소켓, 데이터 참조)
- [ ] `WeaponPistol` / `WeaponKnife` 파생 또는 BP
- [ ] 캐릭터 슬롯: 권총·칼 보유 및 `CurrentWeapon` 전환
- [ ] 입력: 장착 전환(`IA_EquipPistol` 등) → 실제 장착 로직 연결
- [ ] 1P/3P(또는 원격용) 메쉬 부착 규칙 정리 (소켓 이름 통일)
- [ ] 네트워크: 무기 액터/장착 상태 복제 또는 RPC 설계
- [ ] 권총: 히트스캔 또는 투사체 중 하나로 MVP 확정 후 구현
- [ ] 칼: 근접 Trace 범위·쿨다운·방향(카메라 기준) 정리

### Phase 4 - 전투 액션 (GAS)
- [ ] `GA_WeaponEquip` (또는 장착 전용 Ability)
- [ ] `GA_WeaponFirePistol` — 서버 권한 판정 + 탄약(있다면) 소비
- [ ] `GA_WeaponAttackKnife` — 서버 근접 판정
- [ ] `GE_Damage` — 체력 감소, `Health` Attribute와 연동
- [ ] GameplayTag: `State.Attacking`, `State.Reloading` 등 충돌 방지
- [ ] 피격 피드백(선택): 카메라/사운드는 클라, 데미지는 서버

### Phase 5 - 애니메이션 / 동기화
- [ ] ABP: 하체 로코모션 + 상체 레이어(또는 슬롯) 구조
- [ ] 권총: Fire / Reload / Equip 몽타주 연결
- [ ] 칼: Equip / Melee 몽타주 연결
- [ ] Ability 타이밍과 몽타주(Notify) 정렬
- [ ] 멀티: 원격 플레이어 무기·상태가 보이도록 애님/복제 점검
- [ ] (에셋 있으면) MuzzleFlash·히트 이펙트 스폰 위치 정리

### Phase 6 - 데이터 분리 / 밸런싱
- [ ] 무기별 `DataAsset` 또는 `DataTable` (데미지, 연사, 사거리, 탄창 등)
- [ ] 캐릭터별 기본 스탯 테이블 (에이전트 A/B)
- [ ] `GE_DefaultAttributes`를 데이터 기반으로만 채우도록 정리
- [ ] 코드 수정 없이 수치 조정 가능한지 한 번 검증

## 다음 작업 (우선순위)

1. BP에서 `ABaseFPSCharacter` 팔 메쉬 세팅
2. `GE_DefaultAttributes` 적용 및 에디터 에셋 연결
3. Phase 3: 무기 베이스(권총/칼) C++ 또는 BP

## 에셋 준비 리스트 (나중에 확보)

Fab / 언리얼 마켓 등에서 살 때 **스켈레톤 호환(리타깅 여부), LOD, 소켓 이름, 라이선스(상업/수정)** 를 꼭 확인한다.

### 1. 캐릭터

- [ ] **전신 메쉬** (3인칭·다른 플레이어에게 보이는 바디)
- [ ] **1인칭 팔/손 메쉬** (또는 전신과 동일 스켈레톤으로 2컴포넌트 구성 가능한지 확인)
- [ ] **스켈레톤·릭** (IK/풋 IK 필요 시 포함 여부)
- [ ] **기본 애니메이션**: Idle, Walk/Run, Jump, Land
- [ ] **무기별 상체**: 권총 Hold/ADS/Fire/Reload, 칼 Equip/Slash (몽타주 또는 시퀀스)
- [ ] **머티리얼·텍스처** (스킨 변형용 MI 분리 가능한지)
- [ ] **에이전트 A/B**용 외형 차이(메쉬 또는 머티리얼 변주)

### 2. Weapon

- [ ] **권총** 스태틱 또는 스켈레탈 메쉬 (손/그립 소켓에 맞는지)
- [ ] **칼(근접)** 메쉬
- [ ] **무기 소켓 이름** 문서화 (`HandGrip_R` 등 프로젝트 표준으로 통일)
- [ ] **발사/재장전/검집** 애니메이션 (권총)
- [ ] **근접 스윙** 애니메이션 (칼)
- [ ] **사운드**: 발사, 재장전, 빈 탄, 근접 스윙/히트 (선택)
- [ ] **아이콘/UI** (인벤·킬로그용, 나중에)

### 3. MuzzleFlash

- [ ] **총구 화염** 나이아가라 또는 스프라이트 시퀀스
- [ ] **연기/열기** (선택, 짧은 버스트)
- [ ] **조명 스팟** 플래시 (1프레임 또는 짧은 페이드, 성능 고려)
- [ ] **탄피 이펙트** (선택)
- [ ] **무기별 변형** (권총 vs 장차기 대비 스케일/색만 바꿀지)