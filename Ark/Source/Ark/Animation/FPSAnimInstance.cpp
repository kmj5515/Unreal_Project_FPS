#include "FPSAnimInstance.h"

#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

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
		bIsInAir = false;
		bIsCrouched = false;
		return;
	}

	const FVector Velocity = OwningCharacter->GetVelocity();
	const FVector HorizontalVelocity(Velocity.X, Velocity.Y, 0.0f);
	Speed = HorizontalVelocity.Size();
	Direction = CalculateDirection(Velocity, OwningCharacter->GetActorRotation());
	bIsCrouched = OwningCharacter->bIsCrouched;

	if (const UCharacterMovementComponent* MovementComponent = OwningCharacter->GetCharacterMovement())
	{
		bIsInAir = MovementComponent->IsFalling();
	}
	else
	{
		bIsInAir = false;
	}
}
