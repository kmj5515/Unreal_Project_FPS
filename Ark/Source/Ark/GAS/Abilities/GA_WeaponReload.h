#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GA_WeaponReload.generated.h"

UCLASS()
class ARK_API UGA_WeaponReload : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UGA_WeaponReload();

protected:
	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;
};
