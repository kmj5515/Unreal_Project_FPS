#include "FPSPlayerState.h"

#include "../GAS/FPSAttributeSet.h"
#include "AbilitySystemComponent.h"

AFPSPlayerState::AFPSPlayerState()
{
	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	AttributeSet = CreateDefaultSubobject<UFPSAttributeSet>(TEXT("AttributeSet"));

	NetUpdateFrequency = 100.f;
}

UAbilitySystemComponent* AFPSPlayerState::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

UFPSAttributeSet* AFPSPlayerState::GetAttributeSet() const
{
	return AttributeSet;
}
