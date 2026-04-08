#include "FPSPlayerState.h"

#include "../GAS/FPSAttributeSet.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"

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

void AFPSPlayerState::TryApplyDefaultAttributes()
{
	if (!HasAuthority() || bDefaultAttributesApplied || !AbilitySystemComponent || !DefaultAttributesEffect)
	{
		return;
	}

	FGameplayEffectContextHandle EffectContext = AbilitySystemComponent->MakeEffectContext();
	EffectContext.AddSourceObject(this);

	const FGameplayEffectSpecHandle SpecHandle =
		AbilitySystemComponent->MakeOutgoingSpec(DefaultAttributesEffect, 1.f, EffectContext);

	if (!SpecHandle.IsValid())
	{
		return;
	}

	AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
	bDefaultAttributesApplied = true;
}
