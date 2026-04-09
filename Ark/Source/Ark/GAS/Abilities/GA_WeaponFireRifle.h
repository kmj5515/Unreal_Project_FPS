#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GA_WeaponFireRifle.generated.h"

UCLASS()
class ARK_API UGA_WeaponFireRifle : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UGA_WeaponFireRifle();

protected:
	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;
};
