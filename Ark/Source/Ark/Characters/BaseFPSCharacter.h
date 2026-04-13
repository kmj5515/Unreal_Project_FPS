#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "BaseFPSCharacter.generated.h"

class UCameraComponent;
class UInputAction;
class UAbilitySystemComponent;
class UFPSAttributeSet;
class UAnimMontage;
class AWeaponBase;
struct FOnAttributeChangeData;
class UFPSHUDWidget;

DECLARE_MULTICAST_DELEGATE_TwoParams(FFPSHUDHealthChangedSignature, float, float);
DECLARE_MULTICAST_DELEGATE_TwoParams(FFPSHUDAmmoChangedSignature, int32, int32);

UENUM(BlueprintType)
enum class EFPSWeaponSlot : uint8
{
	Primary = 0,
	Secondary,
	Melee
};

UCLASS()
class ARK_API ABaseFPSCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ABaseFPSCharacter();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void RequestEquipWeaponSlot(EFPSWeaponSlot Slot);

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void RequestStartFire();

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void RequestStopFire();

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void RequestReload();

	UFUNCTION(BlueprintPure, Category = "Combat")
	bool IsDead() const { return bDead; }

	UFUNCTION(BlueprintPure, Category = "Combat")
	UAnimMontage* GetDeathMontage() const { return DeathMontage; }

	void NotifyReloadStarted();
	void NotifyReloadFinished();
	void NotifyAmmoChanged();
	void NotifyAmmoChangedValues(int32 CurrentInMag, int32 InMagSize);

	UFUNCTION(BlueprintPure, Category = "HUD")
	float GetHealthCurrent() const;

	UFUNCTION(BlueprintPure, Category = "HUD")
	float GetHealthMax() const;

	UFUNCTION(BlueprintPure, Category = "HUD")
	int32 GetAmmoInMag() const;

	UFUNCTION(BlueprintPure, Category = "HUD")
	int32 GetMagSize() const;

	FFPSHUDHealthChangedSignature& OnHUDHealthChanged() { return HUDHealthChanged; }
	FFPSHUDAmmoChangedSignature& OnHUDAmmoChanged() { return HUDAmmoChanged; }

protected:
	virtual void BeginPlay() override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void OnRep_PlayerState() override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void StartJump();
	void StopJump();
	void HandleEquipPrimary();
	void HandleEquipSecondary();
	void HandleEquipMelee();
	void HandleFireStarted();
	void HandleFireStopped();
	void HandleCrouchStarted();
	void HandleCrouchStopped();
	void HandleReloadStarted();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<UCameraComponent> FirstPersonCamera;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera")
	FName CameraAttachSocketName = FName(TEXT("head"));

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera")
	FVector CameraSocketOffset = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera")
	FRotator CameraSocketRotation = FRotator::ZeroRotator;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> LookAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> JumpAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> FireAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> EquipPrimaryAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> EquipSecondaryAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> EquipMeleeAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> CrouchAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> ReloadAction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input", meta = (ClampMin = "0.01", ClampMax = "5.0"))
	float LookSensitivityMultiplier = 0.5f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	void InitializeAbilityActorInfo();
	void OnMoveSpeedChanged(const FOnAttributeChangeData& ChangeData);
	void OnHealthChanged(const FOnAttributeChangeData& ChangeData);
	void BroadcastHUDHealth();
	void BroadcastHUDAmmo();
	void HandleDeathFromAuthority();
	void FreezeDeathCameraIfLocal();
	void ApplyMoveSpeed(float NewMoveSpeed);
	void AttachViewCameraToMesh();
	void SpawnDefaultLoadout();
	void EquipWeaponBySlot(EFPSWeaponSlot Slot);
	void ApplyCurrentWeaponVisibility();

	UFUNCTION(Server, Reliable)
	void ServerEquipWeapon(EFPSWeaponSlot Slot);

	UFUNCTION(Server, Reliable)
	void ServerSetFiring(bool bNewFiring);

	UFUNCTION(Server, Reliable)
	void ServerStartReload();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_OnDeath();

	UFUNCTION()
	void OnRep_CurrentWeapon();

	UFUNCTION()
	void OnRep_PossessedWeapons();

	UFUNCTION()
	void OnRep_Dead();

	UFUNCTION()
	void OnRep_HUDAmmoInMag();

	UFUNCTION()
	void OnRep_HUDMagSize();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	FName WeaponAttachSocketName = FName(TEXT("Weapon"));

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	TSubclassOf<AWeaponBase> PrimaryWeaponClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	TSubclassOf<AWeaponBase> SecondaryWeaponClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
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
	EFPSWeaponSlot CurrentWeaponSlot = EFPSWeaponSlot::Primary;

	UPROPERTY(ReplicatedUsing = OnRep_Dead, BlueprintReadOnly, Category = "Combat")
	bool bDead = false;

	UPROPERTY(ReplicatedUsing = OnRep_HUDAmmoInMag)
	int32 HUDAmmoInMag = 0;

	UPROPERTY(ReplicatedUsing = OnRep_HUDMagSize)
	int32 HUDMagSize = 0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	TObjectPtr<UAnimMontage> DeathMontage;

	UPROPERTY()
	TObjectPtr<UFPSAttributeSet> CachedAttributeSet;

	FDelegateHandle MoveSpeedChangedDelegateHandle;
	FDelegateHandle HealthChangedDelegateHandle;
	FFPSHUDHealthChangedSignature HUDHealthChanged;
	FFPSHUDAmmoChangedSignature HUDAmmoChanged;
};
