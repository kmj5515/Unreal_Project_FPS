#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "BaseFPSCharacter.generated.h"

class UCameraComponent;
class UInputAction;
class UAbilitySystemComponent;
class UFPSAttributeSet;
struct FOnAttributeChangeData;

UCLASS()
class ARK_API ABaseFPSCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ABaseFPSCharacter();

protected:
	virtual void BeginPlay() override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void OnRep_PlayerState() override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void StartJump();
	void StopJump();

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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input", meta = (ClampMin = "0.01", ClampMax = "5.0"))
	float LookSensitivityMultiplier = 0.5f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	void InitializeAbilityActorInfo();
	void OnMoveSpeedChanged(const FOnAttributeChangeData& ChangeData);
	void ApplyMoveSpeed(float NewMoveSpeed);
	void AttachViewCameraToMesh();

	UPROPERTY()
	TObjectPtr<UFPSAttributeSet> CachedAttributeSet;

	FDelegateHandle MoveSpeedChangedDelegateHandle;
};
