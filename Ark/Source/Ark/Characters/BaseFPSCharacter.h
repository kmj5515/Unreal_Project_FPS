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

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> LookAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> JumpAction;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	void InitializeAbilityActorInfo();
	void OnMoveSpeedChanged(const FOnAttributeChangeData& ChangeData);
	void ApplyMoveSpeed(float NewMoveSpeed);

	UPROPERTY()
	TObjectPtr<UFPSAttributeSet> CachedAttributeSet;

	FDelegateHandle MoveSpeedChangedDelegateHandle;
};
