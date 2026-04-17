#include "BaseFPSCharacter.h"

#include "../Core/FPSPlayerState.h"
#include "../GAS/FPSAttributeSet.h"
#include "../GAS/FPSGameplayTags.h"
#include "../Weapons/WeaponBase.h"
#include "AbilitySystemComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SceneComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "EnhancedInputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "InputAction.h"
#include "Net/UnrealNetwork.h"
#include "CoreGlobals.h"

ABaseFPSCharacter::ABaseFPSCharacter()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_GameTraceChannel1, ECR_Ignore);

	bUseControllerRotationPitch = true;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = false;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MaxWalkSpeedCrouched = 250.f;
	GetCharacterMovement()->JumpZVelocity = 520.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;

	CombatComponent = CreateDefaultSubobject<UFPSCombatComponent>(TEXT("CombatComponent"));
	CombatComponent->SetIsReplicated(true);

	FirstPersonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCamera->SetupAttachment(GetCapsuleComponent());
	FirstPersonCamera->SetRelativeLocation(FVector(0.f, 0.f, 64.f));
	FirstPersonCamera->bUsePawnControlRotation = true;
}

void ABaseFPSCharacter::RequestEquipWeaponSlot(EFPSWeaponSlot Slot)
{
	if (bDead)
	{
		return;
	}
	if (CombatComponent)
	{
		CombatComponent->RequestEquipWeaponSlot(Slot);
	}
}

void ABaseFPSCharacter::RequestStartFire()
{
	if (bDead)
	{
		return;
	}
	if (CombatComponent)
	{
		CombatComponent->RequestStartFire();
	}
}

void ABaseFPSCharacter::RequestStopFire()
{
	if (bDead)
	{
		return;
	}
	if (CombatComponent)
	{
		CombatComponent->RequestStopFire();
	}
}

void ABaseFPSCharacter::RequestReload()
{
	if (bDead)
	{
		return;
	}
	if (CombatComponent)
	{
		CombatComponent->RequestReload();
	}
}

void ABaseFPSCharacter::RequestPickupOverlappingWeapon()
{
	if (bDead)
	{
		return;
	}

	HandleInteractPressed();
}

void ABaseFPSCharacter::RequestDropCurrentWeapon()
{
	if (bDead)
	{
		return;
	}

	HandleDropPressed();
}

void ABaseFPSCharacter::SetOverlappingWeapon(AWeaponBase* InWeapon)
{
	if (CombatComponent)
	{
		CombatComponent->SetOverlappingWeapon(InWeapon);
	}
}

AWeaponBase* ABaseFPSCharacter::GetOverlappingWeapon() const
{
	return CombatComponent ? CombatComponent->GetOverlappingWeapon() : nullptr;
}

void ABaseFPSCharacter::BeginPlay()
{
	Super::BeginPlay();
	AttachViewCameraToMesh();

	if (UCapsuleComponent* Capsule = GetCapsuleComponent())
	{
		Capsule->SetCollisionResponseToChannel(ECC_GameTraceChannel1, ECR_Ignore);
	}

	TArray<USkeletalMeshComponent*> MeshComponents;
	GetComponents<USkeletalMeshComponent>(MeshComponents);
	for (USkeletalMeshComponent* MeshComp : MeshComponents)
	{
		if (!MeshComp)
		{
			continue;
		}
		MeshComp->SetCollisionResponseToChannel(ECC_GameTraceChannel1, ECR_Block);
	}
}

void ABaseFPSCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	CombatComponent = FindComponentByClass<UFPSCombatComponent>();
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

		if (FireAction)
		{
			EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Started, this, &ABaseFPSCharacter::HandleFireStarted);
			EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Completed, this, &ABaseFPSCharacter::HandleFireStopped);
		}

		if (EquipPrimaryAction)
		{
			EnhancedInputComponent->BindAction(EquipPrimaryAction, ETriggerEvent::Started, this, &ABaseFPSCharacter::HandleEquipPrimary);
		}

		if (EquipSecondaryAction)
		{
			EnhancedInputComponent->BindAction(EquipSecondaryAction, ETriggerEvent::Started, this, &ABaseFPSCharacter::HandleEquipSecondary);
		}

		if (EquipMeleeAction)
		{
			EnhancedInputComponent->BindAction(EquipMeleeAction, ETriggerEvent::Started, this, &ABaseFPSCharacter::HandleEquipMelee);
		}

		if (CrouchAction)
		{
			EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Started, this, &ABaseFPSCharacter::HandleCrouchStarted);
			EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Completed, this, &ABaseFPSCharacter::HandleCrouchStopped);
		}

		if (ReloadAction)
		{
			EnhancedInputComponent->BindAction(ReloadAction, ETriggerEvent::Started, this, &ABaseFPSCharacter::HandleReloadStarted);
		}

		if (PickupAction)
		{
			EnhancedInputComponent->BindAction(PickupAction, ETriggerEvent::Started, this, &ABaseFPSCharacter::HandleInteractPressed);
		}

		if (DropAction)
		{
			EnhancedInputComponent->BindAction(DropAction, ETriggerEvent::Started, this, &ABaseFPSCharacter::HandleDropPressed);
		}
	}
}

void ABaseFPSCharacter::Move(const FInputActionValue& Value)
{
	if (bDead)
	{
		return;
	}

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
	if (bDead)
	{
		return;
	}

	const FVector2D InputAxis = Value.Get<FVector2D>() * LookSensitivityMultiplier;

	if (Controller != nullptr)
	{
		AddControllerYawInput(InputAxis.X);
		AddControllerPitchInput(InputAxis.Y);
	}
}

void ABaseFPSCharacter::StartJump()
{
	if (bDead)
	{
		return;
	}
	Jump();
}

void ABaseFPSCharacter::StopJump()
{
	if (bDead)
	{
		return;
	}
	StopJumping();
}

void ABaseFPSCharacter::HandleEquipPrimary()
{
	if (bDead)
	{
		return;
	}
	if (CombatComponent)
	{
		CombatComponent->HandleEquipPrimary();
	}
}

void ABaseFPSCharacter::HandleEquipSecondary()
{
	if (bDead)
	{
		return;
	}
	if (CombatComponent)
	{
		CombatComponent->HandleEquipSecondary();
	}
}

void ABaseFPSCharacter::HandleEquipMelee()
{
	if (bDead)
	{
		return;
	}
	if (CombatComponent)
	{
		CombatComponent->HandleEquipMelee();
	}
}

void ABaseFPSCharacter::HandleCrouchStarted()
{
	if (bDead)
	{
		return;
	}
	Crouch();
}

void ABaseFPSCharacter::HandleCrouchStopped()
{
	if (bDead)
	{
		return;
	}
	UnCrouch();
}

void ABaseFPSCharacter::HandleFireStarted()
{
	if (bDead)
	{
		return;
	}
	if (CombatComponent)
	{
		CombatComponent->HandleFireStarted();
	}
}

void ABaseFPSCharacter::HandleFireStopped()
{
	if (bDead)
	{
		return;
	}
	if (CombatComponent)
	{
		CombatComponent->HandleFireStopped();
	}
	ConsecutiveRecoilShots = 0;
}

void ABaseFPSCharacter::HandleReloadStarted()
{
	if (bDead)
	{
		return;
	}
	if (CombatComponent)
	{
		CombatComponent->HandleReloadStarted();
	}
}

void ABaseFPSCharacter::HandleInteractPressed()
{
	if (LastInteractInputFrame == GFrameCounter)
	{
		return;
	}
	LastInteractInputFrame = GFrameCounter;

	if (HasAuthority())
	{
		HandleServerInteract();
	}
	else
	{
		ServerInteract();
	}
}

void ABaseFPSCharacter::HandleDropPressed()
{
	if (bDead || LastInteractInputFrame == GFrameCounter)
	{
		return;
	}
	LastInteractInputFrame = GFrameCounter;

	if (CombatComponent)
	{
		CombatComponent->HandleDropCurrentWeapon();
	}
}

void ABaseFPSCharacter::HandleServerInteract()
{
	if (CombatComponent)
	{
		CombatComponent->HandleServerInteract();
	}
}

void ABaseFPSCharacter::ApplyLocalRecoilKick()
{
	if (!IsLocallyControlled())
	{
		return;
	}

	APlayerController* PC = Cast<APlayerController>(Controller);
	if (!PC)
	{
		return;
	}

	++ConsecutiveRecoilShots;
	const int32 SoftShotCount = FMath::Max(1, RecoilSoftShots);
	float ShotMultiplier = RecoilSoftShotMultiplier;
	if (ConsecutiveRecoilShots > SoftShotCount)
	{
		const int32 HardShotIndex = ConsecutiveRecoilShots - SoftShotCount;
		ShotMultiplier = FMath::Clamp(
			1.f + (HardShotIndex * RecoilHardShotStep),
			1.f,
			RecoilHardShotMultiplierMax);
	}

	FRotator ControlRotation = PC->GetControlRotation();
	ControlRotation.Pitch = FMath::ClampAngle(ControlRotation.Pitch + (RecoilPitchKickPerShot * ShotMultiplier), -89.f, 89.f);
	const float YawKick = RecoilYawKickPerShot * ShotMultiplier;
	ControlRotation.Yaw += FMath::FRandRange(-YawKick, YawKick);
	PC->SetControlRotation(ControlRotation);
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

			if (HealthChangedDelegateHandle.IsValid())
			{
				AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
					UFPSAttributeSet::GetHealthAttribute()).Remove(HealthChangedDelegateHandle);
				HealthChangedDelegateHandle.Reset();
			}

			HealthChangedDelegateHandle = AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
				UFPSAttributeSet::GetHealthAttribute()).AddUObject(this, &ABaseFPSCharacter::OnHealthChanged);

			if (FPSPlayerState->HasAuthority())
			{
				FPSPlayerState->TryApplyDefaultAttributes();
			}

			ApplyMoveSpeed(CachedAttributeSet->GetMoveSpeed());
			BroadcastHUDHealth();
			BroadcastHUDAmmo();
		}
	}
}

void ABaseFPSCharacter::OnHealthChanged(const FOnAttributeChangeData& ChangeData)
{
	BroadcastHUDHealth();

	if (bDead)
	{
		return;
	}

	if (ChangeData.NewValue > 0.f || ChangeData.OldValue <= 0.f)
	{
		return;
	}

	if (HasAuthority())
	{
		HandleDeathFromAuthority();
	}
}

void ABaseFPSCharacter::HandleDeathFromAuthority()
{
	if (bDead || !HasAuthority())
	{
		return;
	}

	bDead = true;

	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->AddLooseGameplayTag(FPSGameplayTags::State_Dead.GetTag());
		AbilitySystemComponent->CancelAllAbilities();
	}

	if (CombatComponent)
	{
		CombatComponent->StopCurrentWeaponFire();
	}
	NotifyReloadFinished();
	BroadcastHUDHealth();

	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		Movement->StopMovementImmediately();
		Movement->DisableMovement();
	}

	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		DisableInput(PC);
		PC->SetIgnoreMoveInput(true);
		PC->SetIgnoreLookInput(true);
	}

	Multicast_OnDeath();
}

void ABaseFPSCharacter::FreezeDeathCameraIfLocal()
{
	if (!IsLocallyControlled() || !FirstPersonCamera)
	{
		return;
	}

	FirstPersonCamera->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
	FirstPersonCamera->bUsePawnControlRotation = false;
}

void ABaseFPSCharacter::Multicast_OnDeath_Implementation()
{
	FreezeDeathCameraIfLocal();

	if (UCapsuleComponent* Capsule = GetCapsuleComponent())
	{
		Capsule->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	if (USkeletalMeshComponent* MeshComp = GetMesh())
	{
		if (IsLocallyControlled())
		{
			MeshComp->SetVisibility(true, true);
		}

		if (DeathMontage && MeshComp->GetAnimInstance())
		{
			MeshComp->GetAnimInstance()->Montage_Play(DeathMontage, 1.f);
		}
	}
}

void ABaseFPSCharacter::OnRep_Dead()
{
	if (!bDead)
	{
		return;
	}

	if (CombatComponent)
	{
		CombatComponent->StopCurrentWeaponFire();
	}
	NotifyReloadFinished();

	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		Movement->StopMovementImmediately();
		Movement->DisableMovement();
	}

	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		DisableInput(PC);
		PC->SetIgnoreMoveInput(true);
		PC->SetIgnoreLookInput(true);
	}

	FreezeDeathCameraIfLocal();

	if (UCapsuleComponent* Capsule = GetCapsuleComponent())
	{
		Capsule->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	if (USkeletalMeshComponent* MeshComp = GetMesh())
	{
		if (IsLocallyControlled())
		{
			MeshComp->SetVisibility(true, true);
		}

		if (DeathMontage && MeshComp->GetAnimInstance())
		{
			MeshComp->GetAnimInstance()->Montage_Play(DeathMontage, 1.f);
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

void ABaseFPSCharacter::ServerInteract_Implementation()
{
	if (bDead)
	{
		return;
	}

	HandleServerInteract();
}

void ABaseFPSCharacter::NotifyReloadStarted()
{
	if (!AbilitySystemComponent)
	{
		return;
	}

	AbilitySystemComponent->AddLooseGameplayTag(FPSGameplayTags::State_Reloading.GetTag());
}

void ABaseFPSCharacter::NotifyReloadFinished()
{
	if (!AbilitySystemComponent)
	{
		return;
	}

	AbilitySystemComponent->RemoveLooseGameplayTag(FPSGameplayTags::State_Reloading.GetTag());
}

void ABaseFPSCharacter::NotifyAmmoChanged()
{
	BroadcastHUDAmmo();
}

void ABaseFPSCharacter::NotifyAmmoChangedValues(int32 CurrentInMag, int32 InMagSize)
{
	if (CombatComponent)
	{
		CombatComponent->NotifyAmmoChangedValues(CurrentInMag, InMagSize);
	}
}

void ABaseFPSCharacter::NotifyShotFired()
{
	if (CombatComponent)
	{
		CombatComponent->AddCrosshairShootingImpulse();
	}
	ApplyLocalRecoilKick();
}

float ABaseFPSCharacter::GetHealthCurrent() const
{
	return CachedAttributeSet ? CachedAttributeSet->GetHealth() : 0.f;
}

float ABaseFPSCharacter::GetHealthMax() const
{
	return CachedAttributeSet ? CachedAttributeSet->GetMaxHealth() : 1.f;
}

int32 ABaseFPSCharacter::GetAmmoInMag() const
{
	return CombatComponent ? CombatComponent->GetAmmoInMag() : 0;
}

int32 ABaseFPSCharacter::GetMagSize() const
{
	return CombatComponent ? CombatComponent->GetMagSize() : 0;
}

float ABaseFPSCharacter::GetCrosshairSpread() const
{
	return CombatComponent ? CombatComponent->GetCrosshairSpread() : 0.5f;
}

void ABaseFPSCharacter::BroadcastHUDHealth()
{
	HUDHealthChanged.Broadcast(GetHealthCurrent(), GetHealthMax());
}

void ABaseFPSCharacter::BroadcastHUDAmmo()
{
	HUDAmmoChanged.Broadcast(GetAmmoInMag(), GetMagSize());
}

void ABaseFPSCharacter::BroadcastHUDAmmoDirect(int32 AmmoInMag, int32 MagSize)
{
	HUDAmmoChanged.Broadcast(AmmoInMag, MagSize);
}

void ABaseFPSCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABaseFPSCharacter, bDead);
}
