#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "../Weapons/FPSWeaponTypes.h"
#include "FPSCombatComponent.generated.h"

class ABaseFPSCharacter;
class AWeaponBase;
class AFPSPlayerController;
class AFPSGameHUD;
class UTexture2D;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class ARK_API UFPSCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UFPSCombatComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void SpawnDefaultLoadout();
	void EquipWeaponBySlot(EFPSWeaponSlot Slot);
	void ApplyCurrentWeaponVisibility();
	void SetOverlappingWeapon(AWeaponBase* InWeapon);
	void HandleServerInteract();

	void NotifyAmmoChangedValues(int32 CurrentInMag, int32 InMagSize, int32 ReserveAmmo = 0);
	void NotifyAmmoChanged();

	void RequestEquipWeaponSlot(EFPSWeaponSlot Slot);
	void RequestStartFire();
	void RequestStopFire();
	void RequestReload();

	void HandleEquipPrimary();
	void HandleEquipSecondary();
	void HandleEquipMelee();
	void HandleFireStarted();
	void HandleFireStopped();
	void HandleReloadStarted();
	void HandleDropCurrentWeapon();
	void AddCrosshairShootingImpulse();
	void NotifyLocalShotFiredForDebug();

	void StopCurrentWeaponFire();
	void SetTraceDebugEnabled(bool bEnabled);
	void SetDpsMeasureEnabled(bool bEnabled);
	void GetDpsStats(int32& OutShotCount, float& OutTotalDamage, float& OutElapsedSeconds, float& OutDps) const;
	void SetCurrentWeaponSpread(float NewSpreadDeg);
	float GetCurrentWeaponSpread() const;
	void SetCurrentWeaponAmmoDebug(int32 NewAmmoInMagazine, int32 NewMagazineSize, int32 NewMaxCarryAmmo);
	void GetCurrentWeaponAmmoDebug(int32& OutAmmoInMagazine, int32& OutMagazineSize, int32& OutReserveAmmo, int32& OutMaxCarryAmmo) const;

	AWeaponBase* GetOverlappingWeapon() const { return OverlappingWeapon; }
	AWeaponBase* GetCurrentWeapon() const { return CurrentWeapon; }
	bool HasWeaponInSlot(EFPSWeaponSlot Slot) const;
	bool TryAutoPickupWeaponFromOverlap(AWeaponBase* CandidateWeapon);

	int32 GetAmmoInMag() const { return HUDAmmoInMag; }
	int32 GetMagSize() const { return HUDMagSize; }
	int32 GetAmmoReserve() const { return HUDReserveAmmo; }
	float GetCrosshairSpread() const { return CrosshairSpread; }

protected:
	virtual void BeginPlay() override;

private:
	ABaseFPSCharacter* GetOwningFPSCharacter() const;
	void SetHUDCrosshairs() const;

	UFUNCTION(Server, Reliable)
	void ServerEquipWeapon(EFPSWeaponSlot Slot);

	UFUNCTION(Server, Reliable)
	void ServerSetFiring(bool bNewFiring);

	UFUNCTION(Server, Reliable)
	void ServerStartReload();

	UFUNCTION(Server, Reliable)
	void ServerPickupOverlappingWeapon();

	UFUNCTION(Server, Reliable)
	void ServerDropCurrentWeapon();

	UFUNCTION(Server, Reliable)
	void ServerSetTraceDebugEnabled(bool bEnabled);

	UFUNCTION(Server, Reliable)
	void ServerSetDpsMeasureEnabled(bool bEnabled);

	UFUNCTION(Server, Reliable)
	void ServerSetCurrentWeaponSpread(float NewSpreadDeg);

	UFUNCTION(Server, Reliable)
	void ServerSetCurrentWeaponAmmoDebug(int32 NewAmmoInMagazine, int32 NewMagazineSize, int32 NewMaxCarryAmmo);

	UFUNCTION()
	void OnRep_PossessedWeapons();

	UFUNCTION()
	void OnRep_CurrentWeapon();

	UFUNCTION()
	void OnRep_HUDAmmo();

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	FName WeaponAttachSocketName = FName(TEXT("Weapon"));

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	TSubclassOf<AWeaponBase> PrimaryWeaponClass;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	TSubclassOf<AWeaponBase> SecondaryWeaponClass;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	TSubclassOf<AWeaponBase> MeleeWeaponClass;

	UPROPERTY(ReplicatedUsing = OnRep_PossessedWeapons)
	TObjectPtr<AWeaponBase> PrimaryWeapon;

	UPROPERTY(ReplicatedUsing = OnRep_PossessedWeapons)
	TObjectPtr<AWeaponBase> SecondaryWeapon;

	UPROPERTY(ReplicatedUsing = OnRep_PossessedWeapons)
	TObjectPtr<AWeaponBase> MeleeWeapon;

	UPROPERTY(ReplicatedUsing = OnRep_CurrentWeapon)
	TObjectPtr<AWeaponBase> CurrentWeapon;

	UPROPERTY(Replicated)
	TObjectPtr<AWeaponBase> OverlappingWeapon;

	UPROPERTY(Replicated)
	EFPSWeaponSlot CurrentWeaponSlot = EFPSWeaponSlot::Primary;

	UPROPERTY(ReplicatedUsing = OnRep_HUDAmmo)
	int32 HUDAmmoInMag = 0;

	UPROPERTY(ReplicatedUsing = OnRep_HUDAmmo)
	int32 HUDMagSize = 0;

	UPROPERTY(ReplicatedUsing = OnRep_HUDAmmo)
	int32 HUDReserveAmmo = 0;

	mutable TObjectPtr<AFPSPlayerController> CachedPlayerController;
	mutable TObjectPtr<AFPSGameHUD> CachedHUD;

	float CrosshairVelocityFactor = 0.f;
	float CrosshairInAirFactor = 0.f;
	float CrosshairShootingFactor = 0.f;
	float CrosshairSpread = 0.5f;

	UPROPERTY(EditDefaultsOnly, Category = "HUD|Crosshair")
	TObjectPtr<UTexture2D> DefaultCrosshairCenter;

	UPROPERTY(EditDefaultsOnly, Category = "HUD|Crosshair")
	TObjectPtr<UTexture2D> DefaultCrosshairLeft;

	UPROPERTY(EditDefaultsOnly, Category = "HUD|Crosshair")
	TObjectPtr<UTexture2D> DefaultCrosshairRight;

	UPROPERTY(EditDefaultsOnly, Category = "HUD|Crosshair")
	TObjectPtr<UTexture2D> DefaultCrosshairTop;

	UPROPERTY(EditDefaultsOnly, Category = "HUD|Crosshair")
	TObjectPtr<UTexture2D> DefaultCrosshairBottom;

	UPROPERTY(EditDefaultsOnly, Category = "HUD|Crosshair", meta = (ClampMin = "0.0"))
	float CrosshairInAirMax = 3.0f;

	UPROPERTY(EditDefaultsOnly, Category = "HUD|Crosshair", meta = (ClampMin = "0.0"))
	float CrosshairShootImpulse = 0.35f;

	UPROPERTY(EditDefaultsOnly, Category = "HUD|Crosshair", meta = (ClampMin = "0.0"))
	float CrosshairInAirInterpSpeed = 2.25f;

	UPROPERTY(EditDefaultsOnly, Category = "HUD|Crosshair", meta = (ClampMin = "0.0"))
	float CrosshairGroundInterpSpeed = 30.f;

	UPROPERTY(EditDefaultsOnly, Category = "HUD|Crosshair", meta = (ClampMin = "0.0"))
	float CrosshairShootRecoverInterpSpeed = 2.5f;

	UPROPERTY(EditDefaultsOnly, Category = "HUD|Crosshair", meta = (ClampMin = "0.0"))
	float CrosshairShootingFactorMax = 3.2f;

	UPROPERTY(EditDefaultsOnly, Category = "HUD|Crosshair", meta = (ClampMin = "0.0"))
	float CrosshairShotStackStep = 0.12f;

	UPROPERTY(EditDefaultsOnly, Category = "HUD|Crosshair", meta = (ClampMin = "1.0"))
	float CrosshairShotStackMultiplierMax = 2.2f;

	int32 ConsecutiveShotCount = 0;
	bool bTraceDebugEnabled = false;
	bool bDpsMeasuring = false;
	int32 DpsShotCount = 0;
	float DpsTotalDamage = 0.f;
	float DpsStartTimeSeconds = 0.f;

	void UpdateCrosshairSpread(float DeltaTime);
	void ApplyTraceDebugStateToWeapons() const;
	void ResetDpsStats();

	UTexture2D* ResolveCrosshairTexture(UTexture2D* WeaponTexture, const TObjectPtr<UTexture2D>& SlotDefault) const;
};
