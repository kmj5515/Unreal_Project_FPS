#include "FPSCombatComponent.h"

#include "../Characters/BaseFPSCharacter.h"
#include "../Weapons/WeaponBase.h"
#include "Net/UnrealNetwork.h"

ABaseFPSCharacter* UFPSCombatComponent::GetOwningFPSCharacter() const
{
	return Cast<ABaseFPSCharacter>(GetOwner());
}

UFPSCombatComponent::UFPSCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
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
	ABaseFPSCharacter* OwningChar = GetOwningFPSCharacter();
	if (!OwningChar || OwningChar->IsDead())
	{
		return;
	}

	ServerSetFiring(true);
}

void UFPSCombatComponent::HandleFireStopped()
{
	ABaseFPSCharacter* OwningChar = GetOwningFPSCharacter();
	if (!OwningChar || OwningChar->IsDead())
	{
		return;
	}

	ServerSetFiring(false);
}

void UFPSCombatComponent::HandleReloadStarted()
{
	ABaseFPSCharacter* OwningChar = GetOwningFPSCharacter();
	if (!OwningChar || OwningChar->IsDead())
	{
		return;
	}

	ServerStartReload();
}

void UFPSCombatComponent::HandleDropCurrentWeapon()
{
	ABaseFPSCharacter* OwningChar = GetOwningFPSCharacter();
	if (!OwningChar || OwningChar->IsDead())
	{
		return;
	}

	ServerDropCurrentWeapon();
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
		NotifyAmmoChangedValues(0, 0);
	}
}

void UFPSCombatComponent::NotifyAmmoChangedValues(int32 CurrentInMag, int32 InMagSize)
{
	const int32 NewAmmo = FMath::Max(0, CurrentInMag);
	const int32 NewMagSize = FMath::Max(0, InMagSize);

	if (GetOwner() && GetOwner()->HasAuthority())
	{
		HUDAmmoInMag = NewAmmo;
		HUDMagSize = NewMagSize;
	}

	ABaseFPSCharacter* OwningChar = GetOwningFPSCharacter();
	if (OwningChar)
	{
		OwningChar->BroadcastHUDAmmoDirect(NewAmmo, NewMagSize);
	}
}

void UFPSCombatComponent::NotifyAmmoChanged()
{
	ABaseFPSCharacter* OwningChar = GetOwningFPSCharacter();
	if (OwningChar)
	{
		OwningChar->BroadcastHUDAmmoDirect(HUDAmmoInMag, HUDMagSize);
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
		NotifyAmmoChangedValues(CurrentWeapon->GetCurrentAmmoInMagazine(), CurrentWeapon->GetMagazineSize());
	}
	else
	{
		NotifyAmmoChangedValues(0, 0);
	}
}

void UFPSCombatComponent::OnRep_OverlappingWeapon()
{
}

void UFPSCombatComponent::OnRep_HUDAmmoInMag()
{
	ABaseFPSCharacter* OwningChar = GetOwningFPSCharacter();
	if (OwningChar)
	{
		OwningChar->BroadcastHUDAmmoDirect(HUDAmmoInMag, HUDMagSize);
	}
}

void UFPSCombatComponent::OnRep_HUDMagSize()
{
	ABaseFPSCharacter* OwningChar = GetOwningFPSCharacter();
	if (OwningChar)
	{
		OwningChar->BroadcastHUDAmmoDirect(HUDAmmoInMag, HUDMagSize);
	}
}
