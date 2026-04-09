#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GA_WeaponAttackKnife.generated.h"

UCLASS()
class ARK_API UGA_WeaponAttackKnife : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UGA_WeaponAttackKnife();

protected:
	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;
};
