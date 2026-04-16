#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "../Weapons/FPSWeaponTypes.h"
#include "../Components/FPSCombatComponent.h"
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

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void RequestPickupOverlappingWeapon();

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void RequestDropCurrentWeapon();

	void SetOverlappingWeapon(AWeaponBase* InWeapon);
	AWeaponBase* GetOverlappingWeapon() const;

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

	UFUNCTION(BlueprintPure, Category = "HUD")
	float GetCrosshairSpread() const;

	FFPSHUDHealthChangedSignature& OnHUDHealthChanged() { return HUDHealthChanged; }
	FFPSHUDAmmoChangedSignature& OnHUDAmmoChanged() { return HUDAmmoChanged; }

	void BroadcastHUDAmmoDirect(int32 AmmoInMag, int32 MagSize);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	TObjectPtr<UFPSCombatComponent> CombatComponent;

protected:
	virtual void BeginPlay() override;
	virtual void PostInitializeComponents() override;
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
	void HandleInteractPressed();
	void HandleDropPressed();
	void HandleServerInteract();

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

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> PickupAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> DropAction;

	uint64 LastInteractInputFrame = MAX_uint64;

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

	UFUNCTION(Server, Reliable)
	void ServerInteract();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_OnDeath();

	UFUNCTION()
	void OnRep_Dead();

	UPROPERTY(ReplicatedUsing = OnRep_Dead, BlueprintReadOnly, Category = "Combat")
	bool bDead = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	TObjectPtr<UAnimMontage> DeathMontage;

	UPROPERTY()
	TObjectPtr<UFPSAttributeSet> CachedAttributeSet;

	FDelegateHandle MoveSpeedChangedDelegateHandle;
	FDelegateHandle HealthChangedDelegateHandle;
	FFPSHUDHealthChangedSignature HUDHealthChanged;
	FFPSHUDAmmoChangedSignature HUDAmmoChanged;
};
