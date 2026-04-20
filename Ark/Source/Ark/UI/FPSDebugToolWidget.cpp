#include "FPSDebugToolWidget.h"

#include "../Core/FPSPlayerController.h"
#include "Components/CheckBox.h"
#include "Components/Slider.h"
#include "Components/TextBlock.h"

void UFPSDebugToolWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (CheckBox_HitboxVisible)
	{
		CheckBox_HitboxVisible->OnCheckStateChanged.AddDynamic(this, &UFPSDebugToolWidget::HandleHitboxCheckStateChanged);
	}
	if (CheckBox_TraceVisible)
	{
		CheckBox_TraceVisible->OnCheckStateChanged.AddDynamic(this, &UFPSDebugToolWidget::HandleTraceCheckStateChanged);
	}
	if (CheckBox_DpsMeasure)
	{
		CheckBox_DpsMeasure->OnCheckStateChanged.AddDynamic(this, &UFPSDebugToolWidget::HandleDpsCheckStateChanged);
	}
	if (CheckBox_InfiniteAmmo)
	{
		CheckBox_InfiniteAmmo->OnCheckStateChanged.AddDynamic(this, &UFPSDebugToolWidget::HandleInfiniteAmmoCheckStateChanged);
	}

	RefreshFromController();
	RefreshDpsText();
	RefreshWeaponDebugText();
}

void UFPSDebugToolWidget::NativeDestruct()
{
	if (CheckBox_HitboxVisible)
	{
		CheckBox_HitboxVisible->OnCheckStateChanged.RemoveDynamic(this, &UFPSDebugToolWidget::HandleHitboxCheckStateChanged);
	}
	if (CheckBox_TraceVisible)
	{
		CheckBox_TraceVisible->OnCheckStateChanged.RemoveDynamic(this, &UFPSDebugToolWidget::HandleTraceCheckStateChanged);
	}
	if (CheckBox_DpsMeasure)
	{
		CheckBox_DpsMeasure->OnCheckStateChanged.RemoveDynamic(this, &UFPSDebugToolWidget::HandleDpsCheckStateChanged);
	}
	if (CheckBox_InfiniteAmmo)
	{
		CheckBox_InfiniteAmmo->OnCheckStateChanged.RemoveDynamic(this, &UFPSDebugToolWidget::HandleInfiniteAmmoCheckStateChanged);
	}

	Super::NativeDestruct();
}

void UFPSDebugToolWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	RefreshDpsText();
	RefreshWeaponDebugText();
}

void UFPSDebugToolWidget::BindToPlayerController(AFPSPlayerController* InController)
{
	BoundController = InController;
	RefreshFromController();
}

void UFPSDebugToolWidget::HandleHitboxCheckStateChanged(bool bChecked)
{
	if (BoundController)
	{
		BoundController->SetHitboxDebugEnabled(bChecked);
	}
}

void UFPSDebugToolWidget::HandleTraceCheckStateChanged(bool bChecked)
{
	if (BoundController)
	{
		BoundController->SetTraceDebugEnabled(bChecked);
	}
}

void UFPSDebugToolWidget::HandleDpsCheckStateChanged(bool bChecked)
{
	if (BoundController)
	{
		BoundController->SetDpsMeasureEnabled(bChecked);
	}
}

void UFPSDebugToolWidget::HandleInfiniteAmmoCheckStateChanged(bool bChecked)
{
	if (BoundController)
	{
		BoundController->SetInfiniteAmmoEnabled(bChecked);
	}

	if (Text_InfiniteAmmoState)
	{
		Text_InfiniteAmmoState->SetText(
			bChecked
				? FText::FromString(TEXT("Infinite Ammo: ON"))
				: FText::FromString(TEXT("Infinite Ammo: OFF")));
	}
}

void UFPSDebugToolWidget::RefreshFromController()
{
	if (!BoundController)
	{
		return;
	}

	if (CheckBox_HitboxVisible)
	{
		CheckBox_HitboxVisible->SetIsChecked(BoundController->IsHitboxDebugEnabled());
	}
	if (CheckBox_TraceVisible)
	{
		CheckBox_TraceVisible->SetIsChecked(BoundController->IsTraceDebugEnabled());
	}
	if (CheckBox_DpsMeasure)
	{
		CheckBox_DpsMeasure->SetIsChecked(BoundController->IsDpsMeasuring());
	}
	if (CheckBox_InfiniteAmmo)
	{
		CheckBox_InfiniteAmmo->SetIsChecked(BoundController->IsInfiniteAmmoEnabled());
	}
	if (Text_InfiniteAmmoState)
	{
		Text_InfiniteAmmoState->SetText(
			BoundController->IsInfiniteAmmoEnabled()
				? FText::FromString(TEXT("Infinite Ammo: ON"))
				: FText::FromString(TEXT("Infinite Ammo: OFF")));
	}
}

void UFPSDebugToolWidget::RefreshDpsText()
{
	if (!BoundController)
	{
		return;
	}

	BoundController->GetDpsStats(DpsShotCount, DpsTotalDamage, DpsElapsedSeconds, DpsValue);

	if (Text_DpsValue)
	{
		Text_DpsValue->SetText(FText::FromString(FString::Printf(TEXT("DPS: %.1f"), DpsValue)));
	}

	if (Text_DpsDetail)
	{
		Text_DpsDetail->SetText(
			FText::FromString(
				FString::Printf(TEXT("Shots: %d | Damage: %.1f | Time: %.2fs"), DpsShotCount, DpsTotalDamage, DpsElapsedSeconds)));
	}
}

void UFPSDebugToolWidget::ApplyWeaponSpread(float NewSpreadDeg)
{
	if (!BoundController)
	{
		return;
	}

	BoundController->SetCurrentWeaponSpread(NewSpreadDeg);
	RefreshWeaponDebugText();
}

void UFPSDebugToolWidget::ApplyWeaponAmmo(int32 NewAmmoInMagazine, int32 NewMagazineSize, int32 NewMaxCarryAmmo)
{
	if (!BoundController)
	{
		return;
	}

	BoundController->SetCurrentWeaponAmmoDebug(NewAmmoInMagazine, NewMagazineSize, NewMaxCarryAmmo);
	RefreshWeaponDebugText();
}

void UFPSDebugToolWidget::RefreshWeaponDebugText()
{
	if (!BoundController)
	{
		return;
	}

	CurrentWeaponSpreadDeg = BoundController->GetCurrentWeaponSpread();
	BoundController->GetCurrentWeaponAmmoDebug(
		CurrentWeaponAmmoInMagazine,
		CurrentWeaponMagazineSize,
		CurrentWeaponReserveAmmo,
		CurrentWeaponMaxCarryAmmo);

	if (Text_WeaponSpreadValue)
	{
		Text_WeaponSpreadValue->SetText(
			FText::FromString(FString::Printf(TEXT("SpreadDeg: %.2f"), CurrentWeaponSpreadDeg)));
	}

	if (Slider_WeaponSpread)
	{
		const float CurrentSliderValue = Slider_WeaponSpread->GetValue();
		if (!FMath::IsNearlyEqual(CurrentSliderValue, CurrentWeaponSpreadDeg, KINDA_SMALL_NUMBER))
		{
			Slider_WeaponSpread->SetValue(CurrentWeaponSpreadDeg);
		}
	}

	if (Text_SpreadSliderValue)
	{
		Text_SpreadSliderValue->SetText(
			FText::FromString(FString::Printf(TEXT("Spread: %.2f deg"), CurrentWeaponSpreadDeg)));
	}

	if (Text_WeaponAmmoValue)
	{
		Text_WeaponAmmoValue->SetText(
			FText::FromString(
				FString::Printf(
					TEXT("Ammo %d / %d | Reserve %d | MaxCarry %d"),
					CurrentWeaponAmmoInMagazine,
					CurrentWeaponMagazineSize,
					CurrentWeaponReserveAmmo,
					CurrentWeaponMaxCarryAmmo)));
	}
}
