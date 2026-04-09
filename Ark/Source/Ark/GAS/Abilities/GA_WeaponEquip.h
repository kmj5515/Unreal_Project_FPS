#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "../../Characters/BaseFPSCharacter.h"
#include "GA_WeaponEquip.generated.h"

UCLASS()
class ARK_API UGA_WeaponEquip : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UGA_WeaponEquip();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	EFPSWeaponSlot EquipSlot = EFPSWeaponSlot::Primary;

	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;
};
