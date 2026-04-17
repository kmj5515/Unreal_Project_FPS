#include "FPSHUDWidget.h"

#include "../Characters/BaseFPSCharacter.h"
#include "Components/TextBlock.h"

void UFPSHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();
	RefreshHealthText();
	RefreshAmmoText();
	RefreshKillFeedText();
}

void UFPSHUDWidget::NativeDestruct()
{
	UnbindCharacterDelegates();
	Super::NativeDestruct();
}

void UFPSHUDWidget::BindToCharacter(ABaseFPSCharacter* InCharacter)
{
	if (BoundCharacter == InCharacter)
	{
		return;
	}

	UnbindCharacterDelegates();
	BoundCharacter = InCharacter;
	if (!BoundCharacter)
	{
		return;
	}

	HealthChangedHandle = BoundCharacter->OnHUDHealthChanged().AddUObject(this, &UFPSHUDWidget::HandleHealthChanged);
	AmmoChangedHandle = BoundCharacter->OnHUDAmmoChanged().AddUObject(this, &UFPSHUDWidget::HandleAmmoChanged);

	SetHealth(BoundCharacter->GetHealthCurrent(), BoundCharacter->GetHealthMax());
	SetAmmo(BoundCharacter->GetAmmoInMag(), BoundCharacter->GetMagSize());
}

void UFPSHUDWidget::SetHealth(float Current, float Max)
{
	HealthCurrent = Current;
	HealthMax = FMath::Max(1.f, Max);
	RefreshHealthText();
}

void UFPSHUDWidget::SetAmmo(int32 CurrentInMag, int32 InMagSize)
{
	AmmoInMag = FMath::Max(0, CurrentInMag);
	MagSize = FMath::Max(0, InMagSize);
	RefreshAmmoText();
}

void UFPSHUDWidget::AddKillLogEntry(const FString& KillerName, const FString& VictimName, const FString& WeaponName)
{
	const FString SanitizedKiller = KillerName.IsEmpty() ? TEXT("Unknown") : KillerName;
	const FString SanitizedVictim = VictimName.IsEmpty() ? TEXT("Unknown") : VictimName;
	const FString SanitizedWeapon = WeaponName.IsEmpty() ? TEXT("Unknown") : WeaponName;
	const FString Entry = FString::Printf(TEXT("%s -> %s (%s)"), *SanitizedKiller, *SanitizedVictim, *SanitizedWeapon);

	KillFeedEntries.Insert(Entry, 0);

	const int32 MaxEntries = FMath::Max(1, MaxKillFeedEntries);
	while (KillFeedEntries.Num() > MaxEntries)
	{
		KillFeedEntries.Pop();
	}

	RefreshKillFeedText();
}

void UFPSHUDWidget::RefreshHealthText()
{
	if (TextBlock_CurrentHealth)
	{
		TextBlock_CurrentHealth->SetText(FText::AsNumber(FMath::RoundToInt(HealthCurrent)));
	}

	if (TextBlock_MaxHealth)
	{
		TextBlock_MaxHealth->SetText(FText::AsNumber(FMath::RoundToInt(HealthMax)));
	}
}

void UFPSHUDWidget::RefreshAmmoText()
{
	if (TextBlock_AmmoCurrent)
	{
		TextBlock_AmmoCurrent->SetText(FText::AsNumber(AmmoInMag));
	}

	if (TextBlock_AmmoMax)
	{
		TextBlock_AmmoMax->SetText(FText::AsNumber(MagSize));
	}
}

void UFPSHUDWidget::RefreshKillFeedText()
{
	if (!TextBlock_KillFeed)
	{
		return;
	}

	FString CombinedText;
	for (int32 Index = 0; Index < KillFeedEntries.Num(); ++Index)
	{
		if (Index > 0)
		{
			CombinedText.Append(TEXT("\n"));
		}
		CombinedText.Append(KillFeedEntries[Index]);
	}

	TextBlock_KillFeed->SetText(FText::FromString(CombinedText));
}

void UFPSHUDWidget::HandleHealthChanged(float Current, float Max)
{
	SetHealth(Current, Max);
}

void UFPSHUDWidget::HandleAmmoChanged(int32 CurrentInMag, int32 InMagSize)
{
	SetAmmo(CurrentInMag, InMagSize);
}

void UFPSHUDWidget::UnbindCharacterDelegates()
{
	if (!IsValid(BoundCharacter))
	{
		BoundCharacter = nullptr;
		return;
	}

	if (HealthChangedHandle.IsValid())
	{
		BoundCharacter->OnHUDHealthChanged().Remove(HealthChangedHandle);
		HealthChangedHandle.Reset();
	}

	if (AmmoChangedHandle.IsValid())
	{
		BoundCharacter->OnHUDAmmoChanged().Remove(AmmoChangedHandle);
		AmmoChangedHandle.Reset();
	}

	BoundCharacter = nullptr;
}

