#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "FPSAnimInstance.generated.h"

class ACharacter;

UCLASS(BlueprintType)
class ARK_API UFPSAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	float Speed = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	float Direction = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	bool bJumping = false;

	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	bool bEnableJump = false;

	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	bool bJumpPressed = false;

	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	bool bCrouching = false;

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	TObjectPtr<ACharacter> OwningCharacter;
};
