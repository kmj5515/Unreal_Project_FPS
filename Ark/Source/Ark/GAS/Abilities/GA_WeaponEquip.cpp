#include "GA_WeaponEquip.h"

#include "../../Characters/BaseFPSCharacter.h"

UGA_WeaponEquip::UGA_WeaponEquip()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

void UGA_WeaponEquip::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	if (!ActorInfo || !ActorInfo->AvatarActor.IsValid())
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	ABaseFPSCharacter* Character = Cast<ABaseFPSCharacter>(ActorInfo->AvatarActor.Get());
	if (Character)
	{
		Character->RequestEquipWeaponSlot(EquipSlot);
	}

	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}
