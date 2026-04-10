#include "FPSHUDWidget.h"

#include "../Characters/BaseFPSCharacter.h"
#include "Components/TextBlock.h"

void UFPSHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();
	RefreshHealthText();
	RefreshAmmoText();
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
	if (!BoundCharacter)
	{
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

