#include "FPSAttributeSet.h"

#include "Net/UnrealNetwork.h"

UFPSAttributeSet::UFPSAttributeSet()
{
	InitHealth(100.f);
	InitMaxHealth(100.f);
	InitMoveSpeed(200.f);
}

void UFPSAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UFPSAttributeSet, Health, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UFPSAttributeSet, MaxHealth, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UFPSAttributeSet, MoveSpeed, COND_None, REPNOTIFY_Always);
}

void UFPSAttributeSet::OnRep_Health(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UFPSAttributeSet, Health, OldValue);
}

void UFPSAttributeSet::OnRep_MaxHealth(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UFPSAttributeSet, MaxHealth, OldValue);
}

void UFPSAttributeSet::OnRep_MoveSpeed(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UFPSAttributeSet, MoveSpeed, OldValue);
}
