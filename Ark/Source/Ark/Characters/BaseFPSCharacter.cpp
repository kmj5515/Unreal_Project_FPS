#include "BaseFPSCharacter.h"

#include "../Core/FPSPlayerState.h"
#include "../GAS/FPSAttributeSet.h"
#include "AbilitySystemComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "EnhancedInputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "InputAction.h"

ABaseFPSCharacter::ABaseFPSCharacter()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	bUseControllerRotationPitch = true;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = false;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->JumpZVelocity = 520.f;
	GetCharacterMovement()->AirControl = 0.35f;

	FirstPersonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCamera->SetupAttachment(GetCapsuleComponent());
	FirstPersonCamera->SetRelativeLocation(FVector(0.f, 0.f, 64.f));
	FirstPersonCamera->bUsePawnControlRotation = true;
}

void ABaseFPSCharacter::BeginPlay()
{
	Super::BeginPlay();
	AttachViewCameraToMesh();
}

void ABaseFPSCharacter::AttachViewCameraToMesh()
{
	if (!FirstPersonCamera)
	{
		return;
	}

	USkeletalMeshComponent* MeshComp = GetMesh();
	if (!MeshComp || !MeshComp->GetSkeletalMeshAsset())
	{
		return;
	}

	if (!MeshComp->DoesSocketExist(CameraAttachSocketName))
	{
		return;
	}

	FirstPersonCamera->AttachToComponent(
		MeshComp,
		FAttachmentTransformRules::SnapToTargetNotIncludingScale,
		CameraAttachSocketName);

	FirstPersonCamera->SetRelativeLocation(CameraSocketOffset);
	FirstPersonCamera->SetRelativeRotation(CameraSocketRotation);
}

void ABaseFPSCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	InitializeAbilityActorInfo();
}

void ABaseFPSCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
	InitializeAbilityActorInfo();
}

void ABaseFPSCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent =
		Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (MoveAction)
		{
			EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ABaseFPSCharacter::Move);
		}

		if (LookAction)
		{
			EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ABaseFPSCharacter::Look);
		}

		if (JumpAction)
		{
			EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ABaseFPSCharacter::StartJump);
			EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ABaseFPSCharacter::StopJump);
		}
	}
}

void ABaseFPSCharacter::Move(const FInputActionValue& Value)
{
	const FVector2D InputAxis = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		const FRotator ControlRotation = Controller->GetControlRotation();
		const FRotator YawRotation(0.f, ControlRotation.Yaw, 0.f);

		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		AddMovementInput(ForwardDirection, InputAxis.Y);
		AddMovementInput(RightDirection, InputAxis.X);
	}
}

void ABaseFPSCharacter::Look(const FInputActionValue& Value)
{
	const FVector2D InputAxis = Value.Get<FVector2D>() * LookSensitivityMultiplier;

	if (Controller != nullptr)
	{
		AddControllerYawInput(InputAxis.X);
		AddControllerPitchInput(InputAxis.Y);
	}
}

void ABaseFPSCharacter::StartJump()
{
	Jump();
}

void ABaseFPSCharacter::StopJump()
{
	StopJumping();
}

void ABaseFPSCharacter::InitializeAbilityActorInfo()
{
	if (AFPSPlayerState* FPSPlayerState = GetPlayerState<AFPSPlayerState>())
	{
		AbilitySystemComponent = FPSPlayerState->GetAbilitySystemComponent();
		CachedAttributeSet = FPSPlayerState->GetAttributeSet();
		if (AbilitySystemComponent && CachedAttributeSet)
		{
			AbilitySystemComponent->InitAbilityActorInfo(FPSPlayerState, this);

			if (MoveSpeedChangedDelegateHandle.IsValid())
			{
				AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
					UFPSAttributeSet::GetMoveSpeedAttribute()).Remove(MoveSpeedChangedDelegateHandle);
				MoveSpeedChangedDelegateHandle.Reset();
			}

			MoveSpeedChangedDelegateHandle = AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
				UFPSAttributeSet::GetMoveSpeedAttribute()).AddUObject(this, &ABaseFPSCharacter::OnMoveSpeedChanged);

			ApplyMoveSpeed(CachedAttributeSet->GetMoveSpeed());
		}
	}
}

void ABaseFPSCharacter::OnMoveSpeedChanged(const FOnAttributeChangeData& ChangeData)
{
	ApplyMoveSpeed(ChangeData.NewValue);
}

void ABaseFPSCharacter::ApplyMoveSpeed(float NewMoveSpeed)
{
	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		Movement->MaxWalkSpeed = FMath::Max(0.f, NewMoveSpeed);
	}
}
