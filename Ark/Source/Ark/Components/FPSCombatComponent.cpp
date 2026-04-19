#include "FPSCombatComponent.h"

#include "../Characters/BaseFPSCharacter.h"
#include "../Core/FPSPlayerController.h"
#include "../UI/FPSGameHUD.h"
#include "../Weapons/WeaponBase.h"
#include "Engine/Texture2D.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"

ABaseFPSCharacter* UFPSCombatComponent::GetOwningFPSCharacter() const
{
	return Cast<ABaseFPSCharacter>(GetOwner());
}

UFPSCombatComponent::UFPSCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);
}

void UFPSCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	const ABaseFPSCharacter* OwningChar = GetOwningFPSCharacter();
	if (!OwningChar)
	{
		return;
	}

	UpdateCrosshairSpread(DeltaTime);
	if (OwningChar->IsLocallyControlled())
	{
		SetHUDCrosshairs();
	}
}

void UFPSCombatComponent::BeginPlay()
{
	Super::BeginPlay();

	if (GetOwner() && GetOwner()->HasAuthority())
	{
		SpawnDefaultLoadout();
		ApplyCurrentWeaponVisibility();
	}
}

void UFPSCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UFPSCombatComponent, PrimaryWeapon);
	DOREPLIFETIME(UFPSCombatComponent, SecondaryWeapon);
	DOREPLIFETIME(UFPSCombatComponent, MeleeWeapon);
	DOREPLIFETIME(UFPSCombatComponent, CurrentWeapon);
	DOREPLIFETIME_CONDITION(UFPSCombatComponent, OverlappingWeapon, COND_OwnerOnly);
	DOREPLIFETIME(UFPSCombatComponent, CurrentWeaponSlot);
	DOREPLIFETIME_CONDITION(UFPSCombatComponent, HUDAmmoInMag, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(UFPSCombatComponent, HUDMagSize, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(UFPSCombatComponent, HUDReserveAmmo, COND_OwnerOnly);
}

void UFPSCombatComponent::SpawnDefaultLoadout()
{
	ABaseFPSCharacter* OwningChar = GetOwningFPSCharacter();
	if (!OwningChar || !GetWorld())
	{
		return;
	}

	auto SpawnWeapon = [&](TSubclassOf<AWeaponBase> WeaponClass, EFPSWeaponSlot Slot) -> AWeaponBase*
	{
		if (!WeaponClass)
		{
			return nullptr;
		}

		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = OwningChar;
		SpawnParams.Instigator = OwningChar;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		AWeaponBase* SpawnedWeapon = GetWorld()->SpawnActor<AWeaponBase>(WeaponClass, OwningChar->GetActorTransform(), SpawnParams);
		if (SpawnedWeapon)
		{
			SpawnedWeapon->InitializeWeapon(OwningChar, Slot);
		}

		return SpawnedWeapon;
	};

	PrimaryWeapon = SpawnWeapon(PrimaryWeaponClass, EFPSWeaponSlot::Primary);
	SecondaryWeapon = SpawnWeapon(SecondaryWeaponClass, EFPSWeaponSlot::Secondary);
	MeleeWeapon = SpawnWeapon(MeleeWeaponClass, EFPSWeaponSlot::Melee);
}

void UFPSCombatComponent::EquipWeaponBySlot(EFPSWeaponSlot Slot)
{
	ABaseFPSCharacter* OwningChar = GetOwningFPSCharacter();
	if (!OwningChar || OwningChar->IsDead() || OwningChar->HasAnyFlags(RF_ClassDefaultObject | RF_ArchetypeObject))
	{
		return;
	}

	if (!GetOwner()->HasAuthority())
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
	NotifyAmmoChanged();
	OwningChar->SetIsArmed(true);
}

void UFPSCombatComponent::ApplyCurrentWeaponVisibility()
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

void UFPSCombatComponent::SetOverlappingWeapon(AWeaponBase* InWeapon)
{
	if (GetOwner() && GetOwner()->HasAuthority())
	{
		OverlappingWeapon = InWeapon;
	}
}

bool UFPSCombatComponent::HasWeaponInSlot(EFPSWeaponSlot Slot) const
{
	switch (Slot)
	{
	case EFPSWeaponSlot::Primary:
		return PrimaryWeapon != nullptr;
	case EFPSWeaponSlot::Secondary:
		return SecondaryWeapon != nullptr;
	case EFPSWeaponSlot::Melee:
		return MeleeWeapon != nullptr;
	default:
		return false;
	}
}

bool UFPSCombatComponent::TryAutoPickupWeaponFromOverlap(AWeaponBase* CandidateWeapon)
{
	ABaseFPSCharacter* OwningChar = GetOwningFPSCharacter();
	if (!OwningChar || !GetOwner() || !GetOwner()->HasAuthority() || OwningChar->IsDead()
		|| OwningChar->HasAnyFlags(RF_ClassDefaultObject | RF_ArchetypeObject) || !CandidateWeapon)
	{
		return false;
	}

	// Only auto-pickup if nothing is currently equipped.
	if (CurrentWeapon)
	{
		return false;
	}

	const EFPSWeaponSlot CandidateSlot = CandidateWeapon->GetWeaponSlot();
	if (HasWeaponInSlot(CandidateSlot))
	{
		return false;
	}

	OverlappingWeapon = CandidateWeapon;
	ServerPickupOverlappingWeapon();
	return true;
}

void UFPSCombatComponent::HandleServerInteract()
{
	ABaseFPSCharacter* OwningChar = GetOwningFPSCharacter();
	if (!OwningChar || !GetOwner()->HasAuthority() || OwningChar->IsDead())
	{
		return;
	}

	if (OverlappingWeapon)
	{
		ServerPickupOverlappingWeapon();
		return;
	}

	if (CurrentWeapon)
	{
		ServerDropCurrentWeapon();
	}
}

void UFPSCombatComponent::RequestEquipWeaponSlot(EFPSWeaponSlot Slot)
{
	EquipWeaponBySlot(Slot);
}

void UFPSCombatComponent::RequestStartFire()
{
	HandleFireStarted();
}

void UFPSCombatComponent::RequestStopFire()
{
	HandleFireStopped();
}

void UFPSCombatComponent::RequestReload()
{
	HandleReloadStarted();
}

void UFPSCombatComponent::HandleEquipPrimary()
{
	EquipWeaponBySlot(EFPSWeaponSlot::Primary);
}

void UFPSCombatComponent::HandleEquipSecondary()
{
	EquipWeaponBySlot(EFPSWeaponSlot::Secondary);
}

void UFPSCombatComponent::HandleEquipMelee()
{
	EquipWeaponBySlot(EFPSWeaponSlot::Melee);
}

void UFPSCombatComponent::HandleFireStarted()
{
	ServerSetFiring(true);
}

void UFPSCombatComponent::HandleFireStopped()
{
	ServerSetFiring(false);
	ConsecutiveShotCount = 0;
}

void UFPSCombatComponent::HandleReloadStarted()
{
	ServerStartReload();
}

void UFPSCombatComponent::HandleDropCurrentWeapon()
{
	ServerDropCurrentWeapon();
}

void UFPSCombatComponent::AddCrosshairShootingImpulse()
{
	++ConsecutiveShotCount;
	const float ShotMultiplier = FMath::Clamp(
		1.f + ((ConsecutiveShotCount - 1) * CrosshairShotStackStep),
		1.f,
		CrosshairShotStackMultiplierMax);
	const float ShotImpulse = CrosshairShootImpulse * ShotMultiplier;

	CrosshairShootingFactor = FMath::Clamp(
		CrosshairShootingFactor + ShotImpulse,
		0.f,
		CrosshairShootingFactorMax);
}

void UFPSCombatComponent::StopCurrentWeaponFire()
{
	if (CurrentWeapon)
	{
		CurrentWeapon->StopFire();
	}
}

void UFPSCombatComponent::ServerEquipWeapon_Implementation(EFPSWeaponSlot Slot)
{
	ABaseFPSCharacter* OwningChar = GetOwningFPSCharacter();
	if (!OwningChar || OwningChar->IsDead())
	{
		return;
	}
	EquipWeaponBySlot(Slot);
}

void UFPSCombatComponent::ServerSetFiring_Implementation(bool bNewFiring)
{
	ABaseFPSCharacter* OwningChar = GetOwningFPSCharacter();
	if (!OwningChar || OwningChar->IsDead())
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
		ConsecutiveShotCount = 0;
	}
}

void UFPSCombatComponent::ServerStartReload_Implementation()
{
	ABaseFPSCharacter* OwningChar = GetOwningFPSCharacter();
	if (!OwningChar || OwningChar->IsDead() || !CurrentWeapon)
	{
		return;
	}

	CurrentWeapon->StartReload();
}

void UFPSCombatComponent::ServerPickupOverlappingWeapon_Implementation()
{
	ABaseFPSCharacter* OwningChar = GetOwningFPSCharacter();
	if (!OwningChar || OwningChar->IsDead() || OwningChar->HasAnyFlags(RF_ClassDefaultObject | RF_ArchetypeObject) || !OverlappingWeapon)
	{
		return;
	}

	AWeaponBase* PickedWeapon = OverlappingWeapon;
	OverlappingWeapon = nullptr;
	const EFPSWeaponSlot Slot = PickedWeapon->GetWeaponSlot();

	if (HasWeaponInSlot(Slot))
	{
		return;
	}

	auto DropSlotWeapon = [this](TObjectPtr<AWeaponBase>& SlotWeapon)
	{
		if (!SlotWeapon)
		{
			return;
		}

		SlotWeapon->OnUnequipped();
		SlotWeapon->SetDroppedState(true);
		SlotWeapon = nullptr;
	};

	switch (Slot)
	{
	case EFPSWeaponSlot::Primary:
		if (PrimaryWeapon == CurrentWeapon)
		{
			CurrentWeapon = nullptr;
		}
		DropSlotWeapon(PrimaryWeapon);
		PrimaryWeapon = PickedWeapon;
		break;
	case EFPSWeaponSlot::Secondary:
		if (SecondaryWeapon == CurrentWeapon)
		{
			CurrentWeapon = nullptr;
		}
		DropSlotWeapon(SecondaryWeapon);
		SecondaryWeapon = PickedWeapon;
		break;
	case EFPSWeaponSlot::Melee:
		if (MeleeWeapon == CurrentWeapon)
		{
			CurrentWeapon = nullptr;
		}
		DropSlotWeapon(MeleeWeapon);
		MeleeWeapon = PickedWeapon;
		break;
	default:
		return;
	}
	PickedWeapon->InitializeWeapon(OwningChar, Slot);
	PickedWeapon->SetDroppedState(false);
	EquipWeaponBySlot(Slot);
}

void UFPSCombatComponent::ServerDropCurrentWeapon_Implementation()
{
	ABaseFPSCharacter* OwningChar = GetOwningFPSCharacter();
	if (!OwningChar || OwningChar->IsDead() || !CurrentWeapon)
	{
		return;
	}

	AWeaponBase* WeaponToDrop = CurrentWeapon;
	const EFPSWeaponSlot Slot = CurrentWeaponSlot;
	CurrentWeapon = nullptr;

	switch (Slot)
	{
	case EFPSWeaponSlot::Primary:
		PrimaryWeapon = nullptr;
		break;
	case EFPSWeaponSlot::Secondary:
		SecondaryWeapon = nullptr;
		break;
	case EFPSWeaponSlot::Melee:
		MeleeWeapon = nullptr;
		break;
	default:
		break;
	}

	WeaponToDrop->OnUnequipped();
	WeaponToDrop->SetDroppedState(true);

	if (PrimaryWeapon)
	{
		EquipWeaponBySlot(EFPSWeaponSlot::Primary);
	}
	else if (SecondaryWeapon)
	{
		EquipWeaponBySlot(EFPSWeaponSlot::Secondary);
	}
	else if (MeleeWeapon)
	{
		EquipWeaponBySlot(EFPSWeaponSlot::Melee);
	}
	else
	{
		NotifyAmmoChangedValues(0, 0, 0);
		OwningChar->SetIsArmed(false);
	}
}

void UFPSCombatComponent::NotifyAmmoChangedValues(int32 CurrentInMag, int32 InMagSize, int32 ReserveAmmo)
{
	const int32 NewAmmo = FMath::Max(0, CurrentInMag);
	const int32 NewMagSize = FMath::Max(0, InMagSize);
	const int32 NewReserve = FMath::Max(0, ReserveAmmo);

	if (GetOwner() && GetOwner()->HasAuthority())
	{
		HUDAmmoInMag = NewAmmo;
		HUDMagSize = NewMagSize;
		HUDReserveAmmo = NewReserve;
	}

	ABaseFPSCharacter* OwningChar = GetOwningFPSCharacter();
	if (OwningChar)
	{
		OwningChar->BroadcastHUDAmmoDirect(NewAmmo, NewMagSize, NewReserve);
	}
}

void UFPSCombatComponent::NotifyAmmoChanged()
{
	ABaseFPSCharacter* OwningChar = GetOwningFPSCharacter();
	if (OwningChar)
	{
		OwningChar->BroadcastHUDAmmoDirect(HUDAmmoInMag, HUDMagSize, HUDReserveAmmo);
	}
}

void UFPSCombatComponent::OnRep_PossessedWeapons()
{
	ApplyCurrentWeaponVisibility();
}

void UFPSCombatComponent::OnRep_CurrentWeapon()
{
	ApplyCurrentWeaponVisibility();
	if (CurrentWeapon)
	{
		NotifyAmmoChangedValues(
			CurrentWeapon->GetCurrentAmmoInMagazine(),
			CurrentWeapon->GetMagazineSize(),
			CurrentWeapon->GetReserveAmmo());
	}
	else
	{
		NotifyAmmoChangedValues(0, 0, 0);
	}
}

void UFPSCombatComponent::OnRep_HUDAmmo()
{
	ABaseFPSCharacter* OwningChar = GetOwningFPSCharacter();
	if (OwningChar)
	{
		OwningChar->BroadcastHUDAmmoDirect(HUDAmmoInMag, HUDMagSize, HUDReserveAmmo);
	}
}

void UFPSCombatComponent::UpdateCrosshairSpread(float DeltaTime)
{
	const ABaseFPSCharacter* OwningChar = GetOwningFPSCharacter();
	if (!OwningChar)
	{
		return;
	}

	const UCharacterMovementComponent* MovementComp = OwningChar->GetCharacterMovement();
	if (!MovementComp)
	{
		return;
	}

	FVector Velocity = OwningChar->GetVelocity();
	Velocity.Z = 0.f;
	const FVector2D WalkSpeedRange(0.f, MovementComp->MaxWalkSpeed);
	const FVector2D VelocityMultiplierRange(0.f, 1.f);
	CrosshairVelocityFactor = FMath::GetMappedRangeValueClamped(WalkSpeedRange, VelocityMultiplierRange, Velocity.Size());

	if (MovementComp->IsFalling())
	{
		CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, CrosshairInAirMax, DeltaTime, CrosshairInAirInterpSpeed);
	}
	else
	{
		CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 0.f, DeltaTime, CrosshairGroundInterpSpeed);
	}

	CrosshairShootingFactor = FMath::FInterpTo(CrosshairShootingFactor, 0.f, DeltaTime, CrosshairShootRecoverInterpSpeed);
	if (CrosshairShootingFactor <= KINDA_SMALL_NUMBER)
	{
		ConsecutiveShotCount = 0;
	}

	CrosshairSpread = 0.5f + CrosshairVelocityFactor + CrosshairInAirFactor + CrosshairShootingFactor;
}

void UFPSCombatComponent::SetHUDCrosshairs() const
{
	const ABaseFPSCharacter* OwningChar = GetOwningFPSCharacter();
	if (!OwningChar || !OwningChar->Controller)
	{
		return;
	}

	if (CachedPlayerController == nullptr)
	{
		CachedPlayerController = Cast<AFPSPlayerController>(OwningChar->Controller);
	}
	if (!CachedPlayerController)
	{
		return;
	}

	if (CachedHUD == nullptr)
	{
		CachedHUD = Cast<AFPSGameHUD>(CachedPlayerController->GetHUD());
	}
	if (!CachedHUD)
	{
		return;
	}

	FFPSHUDPackage HUDPackage;
	if (CurrentWeapon)
	{
		HUDPackage.CrosshairsCenter =
			ResolveCrosshairTexture(CurrentWeapon->GetCrosshairCenter(), DefaultCrosshairCenter);
		HUDPackage.CrosshairsLeft =
			ResolveCrosshairTexture(CurrentWeapon->GetCrosshairLeft(), DefaultCrosshairLeft);
		HUDPackage.CrosshairsRight =
			ResolveCrosshairTexture(CurrentWeapon->GetCrosshairRight(), DefaultCrosshairRight);
		HUDPackage.CrosshairsTop =
			ResolveCrosshairTexture(CurrentWeapon->GetCrosshairTop(), DefaultCrosshairTop);
		HUDPackage.CrosshairsBottom =
			ResolveCrosshairTexture(CurrentWeapon->GetCrosshairBottom(), DefaultCrosshairBottom);
	}

	HUDPackage.CrosshairSpread = CrosshairSpread;
	HUDPackage.CrosshairsColor = FLinearColor::White;
	CachedHUD->SetHUDPackage(HUDPackage);
}

UTexture2D* UFPSCombatComponent::ResolveCrosshairTexture(
	UTexture2D* WeaponTexture,
	const TObjectPtr<UTexture2D>& SlotDefault) const
{
	if (WeaponTexture)
	{
		return WeaponTexture;
	}
	return SlotDefault.Get();
}
