#include "GA_WeaponReload.h"

#include "../../Characters/BaseFPSCharacter.h"
#include "../FPSGameplayTags.h"

UGA_WeaponReload::UGA_WeaponReload()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	ActivationOwnedTags.AddTag(FPSGameplayTags::State_Reloading);
	ActivationBlockedTags.AddTag(FPSGameplayTags::State_Attacking);
	ActivationBlockedTags.AddTag(FPSGameplayTags::State_Dead);
	BlockAbilitiesWithTag.AddTag(FPSGameplayTags::State_Attacking);
}

void UGA_WeaponReload::ActivateAbility(
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
		Character->RequestReload();
	}

	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}
