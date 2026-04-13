# 무기 추가 가이드 (Ark)

이 문서는 현재 `Ark` 프로젝트 구조 기준으로 신규 무기를 추가하는 방법을 정리한다.

---

## 0) 현재 구조 요약

- 공통 베이스: `AWeaponBase`
- 슬롯: `Primary`, `Secondary`, `Melee`
- 캐릭터 장착 클래스:
  - `ABaseFPSCharacter::PrimaryWeaponClass`
  - `ABaseFPSCharacter::SecondaryWeaponClass`
  - `ABaseFPSCharacter::MeleeWeaponClass`
- 발사 모드:
  - 히트스캔 (`EFPSFireMode::HitScan`)
  - 발사체 (`EFPSFireMode::Projectile`)
- 발사체 클래스: `AFPSProjectileBullet`

---

## 1) C++ 무기 클래스 만들기

`Weapons` 폴더에 `XXXWeapon.h/.cpp` 추가:

- 헤더: `class ARK_API AXXXWeapon : public AWeaponBase`
- CPP 생성자에서 기본값 지정:
  - `WeaponSlot`
  - `Damage`, `Range`, `RefireRate`, `MagazineSize`
  - `bFullAuto`
  - `FireMode`
  - `ProjectileInitialSpeed` (발사체 모드일 때)

예시(스나이퍼):

```cpp
ASniperWeapon::ASniperWeapon()
{
    WeaponSlot = EFPSWeaponSlot::Primary;
    Damage = 80.f;
    Range = 30000.f;
    RefireRate = 1.0f;
    bFullAuto = false;
    MagazineSize = 5;
    FireMode = EFPSFireMode::Projectile;
    ProjectileInitialSpeed = 20000.f;
}
```

---

## 2) DataAsset 세팅

`UWeaponDataAsset`를 사용하면 코드 기본값을 에셋 값으로 덮어쓴다.

주요 필드:

- `Damage`
- `Range`
- `RefireRate`
- `MagazineSize`
- `bFullAuto`
- `bUseProjectileFire`
- `ProjectileInitialSpeed`
- `ReloadMontage`
- FX/SFX (`MuzzleFlashParticle`, `FireSound` 등)

주의:

- `AWeaponBase::ApplyWeaponDataFromAsset()`가 런타임에 값을 적용하므로, 최종 수치는 DataAsset 기준으로 결정된다.

---

## 3) 블루프린트 무기(BP) 만들기

1. `ASniperWeapon`(또는 추가한 C++ 무기) 기반 BP 생성  
2. BP에서 아래 확인
   - 무기 메시 설정
   - `WeaponData` 연결
   - 소켓 이름 확인 (`MuzzleFlash`, `AmmoEject`)
   - 필요 시 `ProjectileClass`/FX override

---

## 4) 캐릭터에 장착(기본 로드아웃)

캐릭터 BP `Class Defaults`에서 설정:

- `PrimaryWeaponClass`
- `SecondaryWeaponClass`
- `MeleeWeaponClass`

중요:

- `PrimaryWeaponClass` 등은 `EditDefaultsOnly`이므로, 레벨 인스턴스가 아니라 **클래스 기본값(Class Defaults)**에서 넣어야 한다.

---

## 5) 월드 픽업/드랍으로 테스트

현재 입력 규칙:

- `OverlappingWeapon`이 있으면 -> Pickup
- 없고 `CurrentWeapon`이 있으면 -> Drop

키 매핑 예시:

- `E`: PickupAction
- `G`: DropAction

같은 키를 써도 분기 로직이 있지만, 운영은 키 분리를 권장한다.

---

## 6) 발사체 트레이서(Trace/Tracer) 설정

`AFPSProjectileBullet`에 트레이서 파티클 변수:

- `TraceParticle`

동작:

- `BeginPlay()`에서 `TraceParticle`이 있으면 `SpawnEmitterAttached`로 생성된다.

에디터에서 할 일:

1. 발사체 BP를 만들거나 기본 발사체 클래스 설정 확인
2. `TraceParticle`에 원하는 트레이서 파티클 할당

---

## 7) 체크리스트

- [ ] C++ 클래스 컴파일 성공
- [ ] 무기 BP 부모 클래스 확인
- [ ] WeaponData 연결
- [ ] 소켓(`MuzzleFlash`, `AmmoEject`) 확인
- [ ] 캐릭터 Class Defaults에 무기 클래스 할당
- [ ] 입력 매핑(Pickup/Drop) 확인
- [ ] PIE 멀티 테스트(서버/클라)

---

## 8) 트러블슈팅

### Q1. `PrimaryWeaponClass`에 BP가 안 들어감

- `Class Defaults`가 아닌 인스턴스에서 넣으려 한 경우가 많다.
- BP 컴파일 실패 상태인지 확인.
- C++ 변경 직후면 에디터 재시작 후 다시 시도.

### Q2. 발사체는 나가는데 데미지가 안 들어감

- 충돌 채널/콜리전 프리셋 확인.
- 서버 권한에서 `OnProjectileHit`가 호출되는지 로그 확인.

### Q3. 트레이서가 안 보임

- `TraceParticle` 미할당 여부 확인.
- 파티클 에셋의 스폰 공간/스케일 확인.
