#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GA_WeaponFireBase.generated.h"

UCLASS(Abstract)
class ARK_API UGA_WeaponFireBase : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UGA_WeaponFireBase();

protected:
	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;
};
