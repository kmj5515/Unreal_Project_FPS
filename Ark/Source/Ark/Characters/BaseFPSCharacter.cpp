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
	GetCharacterMovement()->MaxWalkSpeedCrouched = 250.f;
	GetCharacterMovement()->JumpZVelocity = 520.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;

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
	EquipWeaponBySlot(Slot);
}

void ABaseFPSCharacter::RequestStartFire()
{
	if (bDead)
	{
		return;
	}
	HandleFireStarted();
}

void ABaseFPSCharacter::RequestStopFire()
{
	if (bDead)
	{
		return;
	}
	HandleFireStopped();
}

void ABaseFPSCharacter::RequestReload()
{
	if (bDead)
	{
		return;
	}
	HandleReloadStarted();
}

void ABaseFPSCharacter::BeginPlay()
{
	Super::BeginPlay();
	AttachViewCameraToMesh();

	if (HasAuthority())
	{
		SpawnDefaultLoadout();
		ApplyCurrentWeaponVisibility();
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

		if (CrouchAction)
		{
			EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Started, this, &ABaseFPSCharacter::HandleCrouchStarted);
			EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Completed, this, &ABaseFPSCharacter::HandleCrouchStopped);
		}

		if (ReloadAction)
		{
			EnhancedInputComponent->BindAction(ReloadAction, ETriggerEvent::Started, this, &ABaseFPSCharacter::HandleReloadStarted);
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
	EquipWeaponBySlot(EFPSWeaponSlot::Primary);
}

void ABaseFPSCharacter::HandleEquipSecondary()
{
	if (bDead)
	{
		return;
	}
	EquipWeaponBySlot(EFPSWeaponSlot::Secondary);
}

void ABaseFPSCharacter::HandleEquipMelee()
{
	if (bDead)
	{
		return;
	}
	EquipWeaponBySlot(EFPSWeaponSlot::Melee);
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
	if (HasAuthority())
	{
		ServerSetFiring(true);
		return;
	}

	ServerSetFiring(true);
}

void ABaseFPSCharacter::HandleFireStopped()
{
	if (bDead)
	{
		return;
	}
	if (HasAuthority())
	{
		ServerSetFiring(false);
		return;
	}

	ServerSetFiring(false);
}

void ABaseFPSCharacter::HandleReloadStarted()
{
	if (bDead)
	{
		return;
	}

	ServerStartReload();
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

	if (CurrentWeapon)
	{
		CurrentWeapon->StopFire();
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
	if (bDead)
	{
		return;
	}

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
	BroadcastHUDAmmo();
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
	if (bDead)
	{
		return;
	}
	EquipWeaponBySlot(Slot);
}

void ABaseFPSCharacter::ServerSetFiring_Implementation(bool bNewFiring)
{
	if (bDead)
	{
		return;
	}

	if (!CurrentWeapon)
	{
		return;
	}

	if (CurrentWeapon->IsReloading() && bNewFiring)
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

void ABaseFPSCharacter::ServerStartReload_Implementation()
{
	if (bDead || !CurrentWeapon)
	{
		return;
	}

	CurrentWeapon->StartReload();
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
	const int32 NewAmmo = FMath::Max(0, CurrentInMag);
	const int32 NewMagSize = FMath::Max(0, InMagSize);

	if (HasAuthority())
	{
		HUDAmmoInMag = NewAmmo;
		HUDMagSize = NewMagSize;
	}

	HUDAmmoChanged.Broadcast(NewAmmo, NewMagSize);
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
	return HUDAmmoInMag;
}

int32 ABaseFPSCharacter::GetMagSize() const
{
	return HUDMagSize;
}

void ABaseFPSCharacter::BroadcastHUDHealth()
{
	HUDHealthChanged.Broadcast(GetHealthCurrent(), GetHealthMax());
}

void ABaseFPSCharacter::BroadcastHUDAmmo()
{
	HUDAmmoChanged.Broadcast(GetAmmoInMag(), GetMagSize());
}

void ABaseFPSCharacter::OnRep_PossessedWeapons()
{
	ApplyCurrentWeaponVisibility();
}

void ABaseFPSCharacter::OnRep_CurrentWeapon()
{
	ApplyCurrentWeaponVisibility();
	if (CurrentWeapon)
	{
		NotifyAmmoChangedValues(CurrentWeapon->GetCurrentAmmoInMagazine(), CurrentWeapon->GetMagazineSize());
	}
	else
	{
		NotifyAmmoChangedValues(0, 0);
	}
}

void ABaseFPSCharacter::OnRep_HUDAmmoInMag()
{
	HUDAmmoChanged.Broadcast(HUDAmmoInMag, HUDMagSize);
}

void ABaseFPSCharacter::OnRep_HUDMagSize()
{
	HUDAmmoChanged.Broadcast(HUDAmmoInMag, HUDMagSize);
}

void ABaseFPSCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABaseFPSCharacter, PrimaryWeapon);
	DOREPLIFETIME(ABaseFPSCharacter, SecondaryWeapon);
	DOREPLIFETIME(ABaseFPSCharacter, MeleeWeapon);
	DOREPLIFETIME(ABaseFPSCharacter, CurrentWeapon);
	DOREPLIFETIME(ABaseFPSCharacter, CurrentWeaponSlot);
	DOREPLIFETIME(ABaseFPSCharacter, bDead);
	DOREPLIFETIME(ABaseFPSCharacter, HUDAmmoInMag);
	DOREPLIFETIME(ABaseFPSCharacter, HUDMagSize);
}
