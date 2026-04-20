#include "FPSHUDWidget.h"

#include "../Characters/BaseFPSCharacter.h"
#include "Components/TextBlock.h"
#include "Components/Widget.h"

void UFPSHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();
	RefreshHealthText();
	RefreshAmmoText();
	RefreshKillFeedText();
	RefreshKillStreakText();
}

void UFPSHUDWidget::NativeDestruct()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(KillStreakClearTimerHandle);
	}

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
	SetAmmo(BoundCharacter->GetAmmoInMag(), BoundCharacter->GetMagSize(), BoundCharacter->GetAmmoReserve());
}

void UFPSHUDWidget::SetHealth(float Current, float Max)
{
	HealthCurrent = Current;
	HealthMax = FMath::Max(1.f, Max);
	RefreshHealthText();
}

void UFPSHUDWidget::SetAmmo(int32 CurrentInMag, int32 InMagSize, int32 InReserveAmmo)
{
	AmmoInMag = FMath::Max(0, CurrentInMag);
	MagSize = FMath::Max(0, InMagSize);
	ReserveAmmo = FMath::Max(0, InReserveAmmo);
	RefreshAmmoText();
}

void UFPSHUDWidget::AddKillLogEntry(const FString& KillerName, const FString& VictimName, const FString& WeaponName)
{
	const FString SanitizedKiller = KillerName.IsEmpty() ? TEXT("Unknown") : KillerName;
	const FString SanitizedVictim = VictimName.IsEmpty() ? TEXT("Unknown") : VictimName;
	const FString Entry = FString::Printf(TEXT("%s > %s"), *SanitizedKiller, *SanitizedVictim);

	KillFeedEntries.Insert(Entry, 0);

	const int32 MaxEntries = FMath::Max(1, MaxKillFeedEntries);
	while (KillFeedEntries.Num() > MaxEntries)
	{
		KillFeedEntries.Pop();
	}

	RefreshKillFeedText();
}

void UFPSHUDWidget::ShowKillStreakAnnouncement(int32 KillStreakCount)
{
	FString NewText;
	int32 NewPriority = 0;
	float NewDuration = 0.f;
	if (!ResolveKillStreakPresentation(KillStreakCount, NewText, NewPriority, NewDuration))
	{
		return;
	}

	// Do not downgrade an on-screen higher tier announcement.
	if (ActiveKillStreakPriority > NewPriority)
	{
		return;
	}

	ActiveKillStreakPriority = NewPriority;
	CurrentKillStreakText = NewText;
	RefreshKillStreakText();

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(KillStreakClearTimerHandle);
		World->GetTimerManager().SetTimer(
			KillStreakClearTimerHandle,
			this,
			&UFPSHUDWidget::ClearKillStreakAnnouncement,
			FMath::Max(0.1f, NewDuration),
			false);
	}
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
	const bool bHasWeapon = (MagSize > 0);
	const ESlateVisibility AmmoVisibility = bHasWeapon ? ESlateVisibility::Visible : ESlateVisibility::Collapsed;
	if (TextBlock_AmmoCurrent)
	{
		TextBlock_AmmoCurrent->SetVisibility(AmmoVisibility);
	}
	if (TextBlock_AmmoMax)
	{
		TextBlock_AmmoMax->SetVisibility(AmmoVisibility);
	}
	if (!bHasWeapon)
	{
		return;
	}

	const int32 Denominator = (ReserveAmmo > 0) ? ReserveAmmo : MagSize;
	const FText Combined = FText::FromString(FString::Printf(TEXT("%d / %d"), AmmoInMag, Denominator));

	if (TextBlock_AmmoCurrent && TextBlock_AmmoMax)
	{
		TextBlock_AmmoCurrent->SetText(FText::AsNumber(AmmoInMag));
		TextBlock_AmmoMax->SetText(FText::AsNumber(Denominator));
	}
	else if (TextBlock_AmmoCurrent)
	{
		TextBlock_AmmoCurrent->SetText(Combined);
	}
	else if (TextBlock_AmmoMax)
	{
		TextBlock_AmmoMax->SetText(Combined);
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

void UFPSHUDWidget::ClearKillStreakAnnouncement()
{
	ActiveKillStreakPriority = 0;
	CurrentKillStreakText.Empty();
	RefreshKillStreakText();
}

void UFPSHUDWidget::RefreshKillStreakText()
{
	if (!TextBlock_KillStreak)
	{
		return;
	}

	if (CurrentKillStreakText.IsEmpty())
	{
		TextBlock_KillStreak->SetVisibility(ESlateVisibility::Collapsed);
		TextBlock_KillStreak->SetText(FText::GetEmpty());
		return;
	}

	TextBlock_KillStreak->SetVisibility(ESlateVisibility::Visible);
	TextBlock_KillStreak->SetText(FText::FromString(CurrentKillStreakText));
}

bool UFPSHUDWidget::ResolveKillStreakPresentation(int32 KillStreakCount, FString& OutText, int32& OutPriority, float& OutDuration) const
{
	OutText.Empty();
	OutPriority = 0;
	OutDuration = 0.f;

	if (KillStreakCount >= 5)
	{
		OutText = TEXT("PENTA KILL");
		OutPriority = 5;
		OutDuration = PentaKillDuration;
		return true;
	}
	if (KillStreakCount == 4)
	{
		OutText = TEXT("QUADRA KILL");
		OutPriority = 4;
		OutDuration = QuadraKillDuration;
		return true;
	}
	if (KillStreakCount == 3)
	{
		OutText = TEXT("MULTI KILL");
		OutPriority = 3;
		OutDuration = MultiKillDuration;
		return true;
	}
	if (KillStreakCount == 2)
	{
		OutText = TEXT("DOUBLE KILL");
		OutPriority = 2;
		OutDuration = DoubleKillDuration;
		return true;
	}

	return false;
}

void UFPSHUDWidget::HandleHealthChanged(float Current, float Max)
{
	SetHealth(Current, Max);
}

void UFPSHUDWidget::HandleAmmoChanged(int32 CurrentInMag, int32 InMagSize, int32 InReserveAmmo)
{
	SetAmmo(CurrentInMag, InMagSize, InReserveAmmo);
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

