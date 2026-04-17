#include "FPSAttributeSet.h"

#include "../Characters/BaseFPSCharacter.h"
#include "GameFramework/Controller.h"
#include "GameFramework/Pawn.h"
#include "Net/UnrealNetwork.h"

UFPSAttributeSet::UFPSAttributeSet()
{
	InitHealth(0.f);
	InitMaxHealth(1.f);
	InitMoveSpeed(0.f);
	InitDamage(0.f);
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

void UFPSAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	if (Attribute == GetMaxHealthAttribute())
	{
		NewValue = FMath::Max(1.f, NewValue);
	}
	else if (Attribute == GetMoveSpeedAttribute())
	{
		NewValue = FMath::Max(0.f, NewValue);
	}
}

void UFPSAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	if (Data.EvaluatedData.Attribute == GetDamageAttribute())
	{
		const float LocalDamage = FMath::Max(0.f, GetDamage());
		SetDamage(0.f);

		if (LocalDamage > 0.f)
		{
			AController* EventInstigator = nullptr;
			AActor* DamageCauser = nullptr;
			const FGameplayEffectContextHandle EffectContext = Data.EffectSpec.GetContext();
			if (EffectContext.IsValid())
			{
				AActor* InstigatorActor = EffectContext.GetOriginalInstigator();
				if (!InstigatorActor)
				{
					InstigatorActor = EffectContext.GetInstigator();
				}

				if (InstigatorActor)
				{
					if (APawn* InstigatorPawn = Cast<APawn>(InstigatorActor))
					{
						EventInstigator = InstigatorPawn->GetController();
					}
					else if (AController* InstigatorController = Cast<AController>(InstigatorActor))
					{
						EventInstigator = InstigatorController;
					}
				}

				if (AActor* EffectCauser = EffectContext.GetEffectCauser())
				{
					DamageCauser = EffectCauser;
				}
				else
				{
					DamageCauser = Cast<AActor>(EffectContext.GetSourceObject());
				}
			}

			if (ABaseFPSCharacter* DamagedCharacter = Cast<ABaseFPSCharacter>(Data.Target.GetAvatarActor()))
			{
				DamagedCharacter->RecordDamageSource(EventInstigator, DamageCauser);
			}

			SetHealth(FMath::Clamp(GetHealth() - LocalDamage, 0.f, GetMaxHealth()));
		}
	}
	else if (Data.EvaluatedData.Attribute == GetHealthAttribute())
	{
		SetHealth(FMath::Clamp(GetHealth(), 0.f, GetMaxHealth()));
	}
	else if (Data.EvaluatedData.Attribute == GetMaxHealthAttribute())
	{
		SetMaxHealth(FMath::Max(1.f, GetMaxHealth()));
		const float CurrentHealth = GetHealth();
		// During initial default-attribute application, Health can be clamped to 1
		// before MaxHealth is updated. Promote it to full health once MaxHealth is valid.
		if (CurrentHealth <= 1.f)
		{
			SetHealth(GetMaxHealth());
		}
		else
		{
			SetHealth(FMath::Clamp(CurrentHealth, 0.f, GetMaxHealth()));
		}
	}
	else if (Data.EvaluatedData.Attribute == GetMoveSpeedAttribute())
	{
		SetMoveSpeed(FMath::Max(0.f, GetMoveSpeed()));
	}
}
