#include "BaseFPSCharacter.h"

#include "../Core/FPSPlayerState.h"
#include "../GAS/FPSAttributeSet.h"
#include "../Weapons/WeaponBase.h"
#include "AbilitySystemComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "EnhancedInputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "InputAction.h"
#include "Net/UnrealNetwork.h"

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

	if (HasAuthority())
	{
		SpawnDefaultLoadout();
		EquipWeaponBySlot(EFPSWeaponSlot::Primary);
	}
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

void ABaseFPSCharacter::HandleEquipPrimary()
{
	EquipWeaponBySlot(EFPSWeaponSlot::Primary);
}

void ABaseFPSCharacter::HandleEquipSecondary()
{
	EquipWeaponBySlot(EFPSWeaponSlot::Secondary);
}

void ABaseFPSCharacter::HandleEquipMelee()
{
	EquipWeaponBySlot(EFPSWeaponSlot::Melee);
}

void ABaseFPSCharacter::HandleFireStarted()
{
	if (HasAuthority())
	{
		ServerSetFiring(true);
		return;
	}

	ServerSetFiring(true);
}

void ABaseFPSCharacter::HandleFireStopped()
{
	if (HasAuthority())
	{
		ServerSetFiring(false);
		return;
	}

	ServerSetFiring(false);
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

			if (FPSPlayerState->HasAuthority())
			{
				FPSPlayerState->TryApplyDefaultAttributes();
			}

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

void ABaseFPSCharacter::SpawnDefaultLoadout()
{
	auto SpawnWeapon = [this](TSubclassOf<AWeaponBase> WeaponClass, EFPSWeaponSlot Slot) -> AWeaponBase*
	{
		if (!WeaponClass)
		{
			return nullptr;
		}

		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.Instigator = this;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		AWeaponBase* SpawnedWeapon = GetWorld()->SpawnActor<AWeaponBase>(WeaponClass, GetActorTransform(), SpawnParams);
		if (SpawnedWeapon)
		{
			SpawnedWeapon->InitializeWeapon(this, Slot);
		}

		return SpawnedWeapon;
	};

	PrimaryWeapon = SpawnWeapon(PrimaryWeaponClass, EFPSWeaponSlot::Primary);
	SecondaryWeapon = SpawnWeapon(SecondaryWeaponClass, EFPSWeaponSlot::Secondary);
	MeleeWeapon = SpawnWeapon(MeleeWeaponClass, EFPSWeaponSlot::Melee);
}

void ABaseFPSCharacter::EquipWeaponBySlot(EFPSWeaponSlot Slot)
{
	if (!HasAuthority())
	{
		ServerEquipWeapon(Slot);
		return;
	}

	AWeaponBase* WeaponToEquip = nullptr;
	switch (Slot)
	{
	case EFPSWeaponSlot::Primary:
		WeaponToEquip = PrimaryWeapon;
		break;
	case EFPSWeaponSlot::Secondary:
		WeaponToEquip = SecondaryWeapon;
		break;
	case EFPSWeaponSlot::Melee:
		WeaponToEquip = MeleeWeapon;
		break;
	default:
		break;
	}

	if (!WeaponToEquip)
	{
		return;
	}

	if (CurrentWeapon == WeaponToEquip)
	{
		return;
	}

	if (CurrentWeapon)
	{
		CurrentWeapon->OnUnequipped();
	}

	CurrentWeapon = WeaponToEquip;
	CurrentWeaponSlot = Slot;
	CurrentWeapon->OnEquipped(WeaponAttachSocketName);
	ApplyCurrentWeaponVisibility();
}

void ABaseFPSCharacter::ApplyCurrentWeaponVisibility()
{
	AWeaponBase* AllWeapons[] = { PrimaryWeapon, SecondaryWeapon, MeleeWeapon };
	for (AWeaponBase* Weapon : AllWeapons)
	{
		if (!Weapon)
		{
			continue;
		}

		const bool bIsCurrent = (Weapon == CurrentWeapon);
		Weapon->SetActorHiddenInGame(!bIsCurrent);
		Weapon->SetActorEnableCollision(bIsCurrent);
	}
}

void ABaseFPSCharacter::ServerEquipWeapon_Implementation(EFPSWeaponSlot Slot)
{
	EquipWeaponBySlot(Slot);
}

void ABaseFPSCharacter::ServerSetFiring_Implementation(bool bNewFiring)
{
	if (!CurrentWeapon)
	{
		return;
	}

	if (bNewFiring)
	{
		CurrentWeapon->StartFire();
	}
	else
	{
		CurrentWeapon->StopFire();
	}
}

void ABaseFPSCharacter::OnRep_CurrentWeapon()
{
	ApplyCurrentWeaponVisibility();
}

void ABaseFPSCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABaseFPSCharacter, PrimaryWeapon);
	DOREPLIFETIME(ABaseFPSCharacter, SecondaryWeapon);
	DOREPLIFETIME(ABaseFPSCharacter, MeleeWeapon);
	DOREPLIFETIME(ABaseFPSCharacter, CurrentWeapon);
	DOREPLIFETIME(ABaseFPSCharacter, CurrentWeaponSlot);
}
