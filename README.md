# Unreal_Project_FPS

멀티 FPS(발로란트/오버워치 스타일) 프로젝트 메인 문서.
상세 진행 내역은 `docs` 하위 문서로 분리 관리한다.

## 운영 원칙 (고정)

1. **무기 개발 관련 작업을 최우선(1순위)으로 고정**한다.
2. 무기 개발과 직접 관련 없는 작업(메뉴/로비/메타 UX/미니맵/일반 연출)은 **전부 후순위**로 둔다.
3. 무기 개발 반복 속도를 높이는 디버그/측정 도구를 우선 제작한다.

## 현재 상태

- 현재 단계: **전투 MVP 완료(Phase 3/4)**, **Phase 5(애니·동기화) 진행 중**
- 반영 완료(핵심): GAS 데미지 파이프라인, 무기 3슬롯/히트스캔·근접, HUD(체력/탄약) 이벤트 갱신, 로컬 플레이어 전용 크로스헤어, 헤드샷 판정 안정화, 사망 기본 로직, 탄피(`AmmoEject`) 배출
- 반영 완료(추가): 탄창+예비탄(`ReserveAmmo` 복제, `MaxCarryAmmo`·`UWeaponDataAsset`), HUD `탄창 / 예비`, 크로스헤어 폴백 텍스처(`UFPSCombatComponent`), `UFPSAttributeSet` Armor/MaxArmor(AC), `AFPSDeathmatchGameMode` 1차(킬 점수·목표 킬 로그), 메뉴 1차 C++ 베이스(`AFPSMainMenuGameMode`, `AFPSMenuPlayerController`, `UFPSMainMenuWidget`, `UFPSLobbyWidget`), **무기 Equip 몽타주(`EquipMontage`)**, **킬/헤드샷/연속킬 사운드 훅(`ClientNotifyKillEvent`)**, **헤드샷 판정 neck/head 확장**, **무기 미장착 시 탄약 텍스트 숨김**
- 잔여: 원격 애니 동기화 보강, AC 밸런스, 피격 피드백(히트마커/방향/사운드 고도화)

## 빠른 링크

- 개발 로그: `docs/DEVLOG/2026-04-07.md`, `docs/DEVLOG/2026-04-08.md`, `docs/DEVLOG/2026-04-09.md`, `docs/DEVLOG/2026-04-10.md`
- 이슈(진행중): `docs/ISSUES/open.md`
- 이슈(해결됨): `docs/ISSUES/resolved.md`
- 무기 추가 가이드: `docs/WEAPONS/add-weapon-guide.md`

## 다음 작업 (무기 개발 1순위)

1. **디버그 UI 프레임 구성:** 키입력으로 열고 닫는 `WBP_DebugTool`(가칭) + `PlayerController` 입력 바인딩
2. **히트박스 토글:** 캐릭터/본 충돌 시각화 토글(로컬/서버 기준 동작 범위 정의 포함)
3. **Trace 토글:** 히트스캔/발사체 트레이스 디버그 드로우 on/off를 UI에서 즉시 변경
4. **무기 DPS 측정기:** 측정 시작/정지, 샷 수·누적 데미지·경과 시간·평균 DPS 표시
5. **무기별 퍼짐값 데이터화:** 무기별 `Spread`/`Crosshair` 값 튜닝 테이블 정리
6. **탄약·멀티 QA:** 드랍/픽업·장전 후 HUD·복제 회귀 테스트
7. **애니·동기화 보강:** 발사/장전/무기 전환/장착(Equip) 원격 시각 일관성 점검
8. **피격 피드백:** 히트마커/헤드샷/피격 방향 UI 및 기본 사운드 고도화
9. **무기별 Idle 애니메이션:** 라이플/권총/스나이퍼 상태별 Idle 분기
10. **AC 밸런스:** `DefaultAttributesEffect`·GameplayEffect로 초기 방어·회복·경감 % 튜닝

## 무기 설계 기준 (운영 중)

### 무기·탄약·드랍/픽업

- 보유 탄약 목표: 라이플 **180발**, 스나이퍼 **30발**, 권총 **48발**
- 무기 최초 생성 시: 해당 무기 기준 **풀 탄약** 스폰
- 멀티 일관성: 드랍 후 픽업 시 `AmmoInMagazine`·`ReserveAmmo` 상태가 서버 권한+복제로 동일하게 유지되어야 함

### HP·AC

- HP: GAS `Health` / `MaxHealth`
- AC: `UFPSAttributeSet` `Armor` / `MaxArmor`, 피해 시 방어 우선 소모

## 진행 요약 (완료/진행)

- [x] 전투 MVP: 라이플/권총/스나이퍼/칼 기본 루프, GAS 데미지/체력
- [x] 무기/입력: 3슬롯 전환, 서버 권한 발사/장전, HUD 체력/탄약 이벤트
- [x] 무기 상호작용: 픽업/드랍 서버 판정, 무기 상태 복제, 드랍 물리 동기화
- [x] 발사체/히트스캔: `ProjectileBullet`, 트레이서, 피격 처리 연결
- [x] 크로스헤어/탄퍼짐: 누적 퍼짐, 반동 연동, 로컬 표시 범위 제한
- [x] 디버그 시각화: 히트스캔/발사체 디버그 드로우(멀티캐스트)
- [x] 탄약: `ReserveAmmo` 복제, `MaxCarryAmmo` 데이터화, 재장전 예비탄 보충
- [x] AC: `UFPSAttributeSet` `Armor`·`MaxArmor` 적용, 피해 시 방어 우선 소모
- [x] 데스매치 1차: `AFPSDeathmatchGameMode`, 킬 점수/목표 킬 로그
- [x] 무기 장착 애니: `AWeaponBase` Equip 몽타주(`EquipMontage`) 지원 및 장착 시 재생
- [x] 킬/헤드샷 사운드: 헤드샷 확정 + 더블/멀티/쿼드라/펜타킬 사운드 훅(`AFPSPlayerController`)
- [x] 연속킬 UI 피드백: 더블/멀티/쿼드라/펜타킬 텍스트, 지속시간, 우선순위 처리(`UFPSHUDWidget`)
- [x] 헤드샷 판정 보강: `head` + `neck` 본 이름 기반 2배 피해 판정(히트스캔/발사체 공통)
- [x] HUD 탄약 가시성: 무기 미장착(`MagSize <= 0`)일 때 탄약 텍스트 숨김
- [~] 애니메이션: 기본 ABP/AnimInstance 적용, 몽타주·원격 동기화 진행 중

## 후순위 백로그 (무기 외)

1. 데스매치 마무리: 매치 종료·승자 표시·맵/메뉴 복귀 규칙 연결
2. 메뉴/로비 플로우 고도화: `CreateGame` / `JoinGame` 등 UX 정리
3. 미니맵: 위젯·월드 미러 방식 확정 후 로컬 플레이어 기준 표시
4. 메타 보상 루프: 레벨·경험치·재화 저장/반영

## 참고 메모

- 메뉴 C++ 베이스는 구현됨. 에디터에서 WBP·맵·GameMode 오버라이드 연결 필요
- 캐릭터 메쉬 이슈가 남아 있어 일부 애니/동기화 작업은 선행 정리가 필요
