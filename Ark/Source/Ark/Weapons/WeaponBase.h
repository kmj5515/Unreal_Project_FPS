#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameFramework/Actor.h"
#include "FPSWeaponTypes.h"
#include "WeaponBase.generated.h"

class USkeletalMeshComponent;
class ABaseFPSCharacter;
class UWorld;
class UGameplayEffect;
class UWeaponDataAsset;
class UParticleSystem;
class USoundBase;
class UAnimMontage;
class UStaticMesh;
class USphereComponent;
class UPrimitiveComponent;
class AFPSProjectileBullet;
class UTexture2D;

UENUM(BlueprintType)
enum class EFPSFireMode : uint8
{
	HitScan = 0,
	Projectile
};

UENUM(BlueprintType)
enum class EFPSWeaponNetState : uint8
{
	Equipped = 0,
	Dropped
};

UCLASS()
class ARK_API AWeaponBase : public AActor
{
	GENERATED_BODY()

public:
	AWeaponBase();

	void InitializeWeapon(ABaseFPSCharacter* InOwnerCharacter, EFPSWeaponSlot InSlot);
	void OnEquipped(const FName& AttachSocketName);
	void OnUnequipped();

	virtual void StartFire();
	virtual void StopFire();
	virtual void StartReload();
	void SetDroppedState(bool bDropped);

	UFUNCTION(BlueprintPure, Category = "Weapon|Ammo")
	bool IsReloading() const { return bIsReloading; }

	UFUNCTION(BlueprintPure, Category = "Weapon|Ammo")
	int32 GetCurrentAmmoInMagazine() const { return AmmoInMagazine; }

	UFUNCTION(BlueprintPure, Category = "Weapon|Ammo")
	int32 GetMagazineSize() const { return MagazineSize; }

	UFUNCTION(BlueprintPure, Category = "Weapon|Ammo")
	int32 GetReserveAmmo() const { return ReserveAmmo; }

	UFUNCTION(BlueprintPure, Category = "Weapon")
	EFPSWeaponSlot GetWeaponSlot() const { return WeaponSlot; }

	UTexture2D* GetCrosshairCenter() const { return CrosshairCenter; }
	UTexture2D* GetCrosshairLeft() const { return CrosshairLeft; }
	UTexture2D* GetCrosshairRight() const { return CrosshairRight; }
	UTexture2D* GetCrosshairTop() const { return CrosshairTop; }
	UTexture2D* GetCrosshairBottom() const { return CrosshairBottom; }
	FString GetKillFeedWeaponName() const;

protected:
	virtual void BeginPlay() override;
	void ApplyWeaponDataFromAsset();

	void FireOnce();
	void ResetFireGate();
	void FinishReload();
	void ApplyCarryAmmoDistribution();
	void BroadcastAmmoToOwner();
	bool CanReload() const;
	bool GetAimStartEnd(FVector& OutStart, FVector& OutEnd) const;
	bool PerformHitscanTrace(FHitResult& OutHit, const FVector& Start, const FVector& End) const;
	void ApplyPointDamageFromHit(const FHitResult& Hit);
	bool TryApplyGasDamageFromHit(const FHitResult& Hit);
	void SpawnProjectile(const FVector& Start, const FVector& End);

	void PerformMeleeAttack();

	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_PlayMuzzleFlash();

	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_DebugHitImpact(const FVector_NetQuantize& ImpactPoint);

	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_DebugShotTrace(
		const FVector_NetQuantize& TraceStart,
		const FVector_NetQuantize& TraceEnd,
		bool bHit,
		const FVector_NetQuantize& ImpactPoint);

	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_PlayFireMontage(UAnimMontage* MontageToPlay);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayReloadMontage(UAnimMontage* MontageToPlay);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_OnReloadFinished();

	UFUNCTION()
	void OnRep_AmmoInMagazine();

	UFUNCTION()
	void OnRep_ReserveAmmo();

	UFUNCTION()
	void OnRep_WeaponState();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	void SetWeaponState(EFPSWeaponNetState NewState);
	void ApplyWeaponState();
	void ApplyEquippedState();
	void ApplyDroppedState();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	TObjectPtr<USkeletalMeshComponent> WeaponMesh;

	UFUNCTION()
	void OnRep_OwnerCharacter();

	UFUNCTION()
	void OnPickupSphereBeginOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

	UFUNCTION()
	void OnPickupSphereEndOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon", ReplicatedUsing = OnRep_OwnerCharacter)
	TObjectPtr<ABaseFPSCharacter> OwnerCharacter;

	UPROPERTY(ReplicatedUsing = OnRep_WeaponState)
	EFPSWeaponNetState WeaponState = EFPSWeaponNetState::Dropped;

	UPROPERTY(Replicated)
	FName EquippedSocketName = FName(TEXT("Weapon"));

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	EFPSWeaponSlot WeaponSlot = EFPSWeaponSlot::Primary;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Stats", meta = (ClampMin = "0.0"))
	float Damage = 25.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Damage")
	TSubclassOf<UGameplayEffect> DamageGameplayEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Damage")
	FGameplayTag DamageSetByCallerTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Stats", meta = (ClampMin = "0.0", Units = "cm"))
	float Range = 10000.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Stats", meta = (ClampMin = "0.01", Units = "s"))
	float RefireRate = 0.12f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Stats", meta = (ClampMin = "1"))
	int32 MagazineSize = 15;

	/** Total ammo cap (mag + reserve). 0 = infinite reserve (reload always fills mag). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Stats", meta = (ClampMin = "0"))
	int32 MaxCarryAmmo = 0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Stats")
	bool bFullAuto = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Stats")
	EFPSFireMode FireMode = EFPSFireMode::HitScan;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Spread")
	bool bUseBulletSpread = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Spread", meta = (ClampMin = "0.0", ClampMax = "15.0"))
	float BulletSpreadPerCrosshairDeg = 0.9f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Melee", meta = (ClampMin = "0.0", Units = "cm"))
	float MeleeRange = 160.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Melee", meta = (ClampMin = "0.0", Units = "cm"))
	float MeleeRadius = 25.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Data")
	TObjectPtr<UWeaponDataAsset> WeaponData;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Trace")
	FName MuzzleSocketName = FName(TEXT("MuzzleFlash"));

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Trace")
	FName AmmoEjectSocketName = FName(TEXT("AmmoEject"));

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|FX")
	TObjectPtr<UParticleSystem> MuzzleFlashParticle;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|FX")
	TObjectPtr<UStaticMesh> ShellEjectStaticMesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|FX", meta = (ClampMin = "0.0", Units = "s"))
	float ShellLifeSpan = 5.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|FX", meta = (ClampMin = "0.0"))
	float ShellImpulseStrength = 120.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|FX", meta = (ClampMin = "0.01"))
	float MuzzleFlashScale = 1.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|SFX")
	TObjectPtr<USoundBase> FireSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Animation")
	TObjectPtr<UAnimMontage> FireMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Animation")
	TObjectPtr<UAnimMontage> ReloadMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|UI")
	FText KillFeedWeaponName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Crosshair")
	TObjectPtr<UTexture2D> CrosshairCenter;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Crosshair")
	TObjectPtr<UTexture2D> CrosshairLeft;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Crosshair")
	TObjectPtr<UTexture2D> CrosshairRight;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Crosshair")
	TObjectPtr<UTexture2D> CrosshairTop;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Crosshair")
	TObjectPtr<UTexture2D> CrosshairBottom;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon|Debug")
	bool bDebugDrawTrace = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon|Debug", meta = (ClampMin = "0.0"))
	float DebugDrawDuration = 1.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon|Pickup")
	TObjectPtr<USphereComponent> PickupSphere;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Projectile")
	TSubclassOf<AFPSProjectileBullet> ProjectileClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Projectile", meta = (ClampMin = "100.0", Units = "cm/s"))
	float ProjectileInitialSpeed = 12000.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Drop", meta = (ClampMin = "0.0"))
	float DropImpulseStrength = 350.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Drop", meta = (ClampMin = "0.0"))
	float SelfPickupBlockAfterDropSeconds = 0.5f;

	UPROPERTY(Transient)
	TObjectPtr<ABaseFPSCharacter> LastDropCharacter;

	UPROPERTY(Transient)
	float LastDropWorldTimeSeconds = -1000.f;

	FTimerHandle ReenablePickupSphereTimerHandle;
	void EnablePickupSphereAfterDropBlock();

	bool bIsFiring = false;
	bool bIsReloading = false;
	FTimerHandle ReloadTimerHandle;
	FTimerHandle RefireTimerHandle;
	FTimerHandle SemiAutoFireGateTimerHandle;

	UPROPERTY(ReplicatedUsing = OnRep_AmmoInMagazine)
	int32 AmmoInMagazine = 15;

	UPROPERTY(ReplicatedUsing = OnRep_ReserveAmmo, BlueprintReadOnly, Category = "Weapon|Ammo")
	int32 ReserveAmmo = 0;
};
