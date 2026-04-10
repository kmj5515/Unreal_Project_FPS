#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameFramework/Actor.h"
#include "../Characters/BaseFPSCharacter.h"
#include "WeaponBase.generated.h"

class USkeletalMeshComponent;
class ABaseFPSCharacter;
class UWorld;
class UGameplayEffect;
class UWeaponDataAsset;
class UParticleSystem;
class USoundBase;
class UAnimMontage;

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

	UFUNCTION(BlueprintPure, Category = "Weapon|Ammo")
	bool IsReloading() const { return bIsReloading; }

	UFUNCTION(BlueprintPure, Category = "Weapon|Ammo")
	int32 GetCurrentAmmoInMagazine() const { return AmmoInMagazine; }

	UFUNCTION(BlueprintPure, Category = "Weapon|Ammo")
	int32 GetMagazineSize() const { return MagazineSize; }

protected:
	virtual void BeginPlay() override;
	void ApplyWeaponDataFromAsset();

	void FireOnce();
	void FinishReload();
	bool CanReload() const;
	bool GetAimStartEnd(FVector& OutStart, FVector& OutEnd) const;
	bool PerformHitscanTrace(FHitResult& OutHit, const FVector& Start, const FVector& End) const;
	void ApplyPointDamageFromHit(const FHitResult& Hit);
	bool TryApplyGasDamageFromHit(const FHitResult& Hit);

	void PerformMeleeAttack();

	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_PlayMuzzleFlash();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayReloadMontage();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_OnReloadFinished();

	UFUNCTION()
	void OnRep_AmmoInMagazine();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	TObjectPtr<USkeletalMeshComponent> WeaponMesh;

	UFUNCTION()
	void OnRep_OwnerCharacter();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon", ReplicatedUsing = OnRep_OwnerCharacter)
	TObjectPtr<ABaseFPSCharacter> OwnerCharacter;

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

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Stats")
	bool bFullAuto = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Melee", meta = (ClampMin = "0.0", Units = "cm"))
	float MeleeRange = 160.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Melee", meta = (ClampMin = "0.0", Units = "cm"))
	float MeleeRadius = 25.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Data")
	TObjectPtr<UWeaponDataAsset> WeaponData;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Trace")
	FName MuzzleSocketName = FName(TEXT("MuzzleFlash"));

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|FX")
	TObjectPtr<UParticleSystem> MuzzleFlashParticle;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|FX", meta = (ClampMin = "0.01"))
	float MuzzleFlashScale = 1.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|SFX")
	TObjectPtr<USoundBase> FireSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Animation")
	TObjectPtr<UAnimMontage> ReloadMontage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon|Debug")
	bool bDebugDrawTrace = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon|Debug", meta = (ClampMin = "0.0"))
	float DebugDrawDuration = 1.0f;

	bool bIsFiring = false;
	bool bIsReloading = false;
	FTimerHandle ReloadTimerHandle;
	FTimerHandle RefireTimerHandle;

	UPROPERTY(ReplicatedUsing = OnRep_AmmoInMagazine)
	int32 AmmoInMagazine = 15;
};
