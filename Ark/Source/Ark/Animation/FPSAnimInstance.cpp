#include "FPSAnimInstance.h"

#include "../Characters/BaseFPSCharacter.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "KismetAnimationLibrary.h"

void UFPSAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
	OwningCharacter = Cast<ACharacter>(TryGetPawnOwner());
}

void UFPSAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (!OwningCharacter)
	{
		OwningCharacter = Cast<ACharacter>(TryGetPawnOwner());
	}

	if (!OwningCharacter)
	{
		Speed = 0.0f;
		Direction = 0.0f;
		bJumping = false;
		bEnableJump = false;
		bJumpPressed = false;
		bCrouching = false;
		bDead = false;
		return;
	}

	bDead = false;
	if (const ABaseFPSCharacter* FPSChar = Cast<ABaseFPSCharacter>(OwningCharacter))
	{
		bDead = FPSChar->IsDead();
	}

	if (bDead)
	{
		Speed = 0.0f;
		Direction = 0.0f;
		bJumping = false;
		bEnableJump = false;
		bJumpPressed = false;
		bCrouching = false;
		return;
	}

	const FVector Velocity = OwningCharacter->GetVelocity();
	const FVector HorizontalVelocity(Velocity.X, Velocity.Y, 0.0f);
	Speed = HorizontalVelocity.Size();
	Direction = UKismetAnimationLibrary::CalculateDirection(Velocity, OwningCharacter->GetActorRotation());
	bCrouching = OwningCharacter->bIsCrouched;

	if (const UCharacterMovementComponent* MovementComponent = OwningCharacter->GetCharacterMovement())
	{
		bJumping = MovementComponent->IsFalling();
	}
	else
	{
		bJumping = false;
	}

	bEnableJump = OwningCharacter->CanJump();
	bJumpPressed = OwningCharacter->bPressedJump;
}
