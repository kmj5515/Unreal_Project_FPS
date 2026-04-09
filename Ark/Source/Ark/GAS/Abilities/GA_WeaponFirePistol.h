#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GA_WeaponFirePistol.generated.h"

UCLASS()
class ARK_API UGA_WeaponFirePistol : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UGA_WeaponFirePistol();

protected:
	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;
};
