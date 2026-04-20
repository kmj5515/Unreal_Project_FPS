#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "FPSDebugToolWidget.generated.h"

class AFPSPlayerController;
class UCheckBox;
class USlider;
class UTextBlock;

UCLASS()
class ARK_API UFPSDebugToolWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	UFUNCTION(BlueprintCallable, Category = "Debug")
	void BindToPlayerController(AFPSPlayerController* InController);

	UFUNCTION(BlueprintCallable, Category = "Debug|Weapon")
	void ApplyWeaponSpread(float NewSpreadDeg);

	UFUNCTION(BlueprintCallable, Category = "Debug|Weapon")
	void ApplyWeaponAmmo(int32 NewAmmoInMagazine, int32 NewMagazineSize, int32 NewMaxCarryAmmo);

protected:
	UFUNCTION()
	void HandleHitboxCheckStateChanged(bool bChecked);

	UFUNCTION()
	void HandleTraceCheckStateChanged(bool bChecked);

	UFUNCTION()
	void HandleDpsCheckStateChanged(bool bChecked);

	UFUNCTION()
	void HandleInfiniteAmmoCheckStateChanged(bool bChecked);

	void RefreshFromController();
	void RefreshDpsText();
	void RefreshWeaponDebugText();

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCheckBox> CheckBox_HitboxVisible;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCheckBox> CheckBox_TraceVisible;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCheckBox> CheckBox_DpsMeasure;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCheckBox> CheckBox_InfiniteAmmo;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> Text_DpsValue;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> Text_DpsDetail;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> Text_WeaponSpreadValue;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<USlider> Slider_WeaponSpread;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> Text_SpreadSliderValue;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> Text_WeaponAmmoValue;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> Text_InfiniteAmmoState;

	UPROPERTY(BlueprintReadOnly, Category = "Debug")
	float DpsValue = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "Debug")
	float DpsElapsedSeconds = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "Debug")
	float DpsTotalDamage = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "Debug")
	int32 DpsShotCount = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Debug|Weapon")
	float CurrentWeaponSpreadDeg = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "Debug|Weapon")
	int32 CurrentWeaponAmmoInMagazine = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Debug|Weapon")
	int32 CurrentWeaponMagazineSize = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Debug|Weapon")
	int32 CurrentWeaponReserveAmmo = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Debug|Weapon")
	int32 CurrentWeaponMaxCarryAmmo = 0;

	TObjectPtr<AFPSPlayerController> BoundController;
};
