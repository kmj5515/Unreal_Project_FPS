#include "GA_WeaponAttackKnife.h"

#include "../../Characters/BaseFPSCharacter.h"
#include "../FPSGameplayTags.h"

UGA_WeaponAttackKnife::UGA_WeaponAttackKnife()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	ActivationOwnedTags.AddTag(FPSGameplayTags::State_Attacking);
	ActivationBlockedTags.AddTag(FPSGameplayTags::State_Reloading);
	BlockAbilitiesWithTag.AddTag(FPSGameplayTags::State_Attacking);
}

void UGA_WeaponAttackKnife::ActivateAbility(
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
		Character->RequestStartFire();
		Character->RequestStopFire();
	}

	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}
