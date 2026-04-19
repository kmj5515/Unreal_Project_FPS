#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "FPSHUDWidget.generated.h"

class UTextBlock;
class ABaseFPSCharacter;

UCLASS()
class ARK_API UFPSHUDWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UFUNCTION(BlueprintCallable, Category = "HUD")
	void BindToCharacter(ABaseFPSCharacter* InCharacter);

	UFUNCTION(BlueprintCallable, Category = "HUD")
	void SetHealth(float Current, float Max);

	UFUNCTION(BlueprintCallable, Category = "HUD")
	void SetAmmo(int32 CurrentInMag, int32 InMagSize, int32 ReserveAmmo = 0);

	UFUNCTION(BlueprintCallable, Category = "HUD")
	void AddKillLogEntry(const FString& KillerName, const FString& VictimName, const FString& WeaponName);

	UPROPERTY(BlueprintReadOnly, Category = "HUD")
	float HealthCurrent = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "HUD")
	float HealthMax = 1.f;

	UPROPERTY(BlueprintReadOnly, Category = "HUD")
	int32 AmmoInMag = 0;

	UPROPERTY(BlueprintReadOnly, Category = "HUD")
	int32 MagSize = 0;

	UPROPERTY(BlueprintReadOnly, Category = "HUD")
	int32 ReserveAmmo = 0;

protected:
	void RefreshHealthText();
	void RefreshAmmoText();
	void RefreshKillFeedText();
	void HandleHealthChanged(float Current, float Max);
	void HandleAmmoChanged(int32 CurrentInMag, int32 InMagSize, int32 InReserveAmmo);
	void UnbindCharacterDelegates();

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> TextBlock_CurrentHealth;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> TextBlock_MaxHealth;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> TextBlock_AmmoCurrent;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> TextBlock_AmmoMax;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> TextBlock_KillFeed;

	UPROPERTY(EditDefaultsOnly, Category = "HUD|KillFeed", meta = (ClampMin = "1", ClampMax = "12"))
	int32 MaxKillFeedEntries = 5;

	UPROPERTY(BlueprintReadOnly, Category = "HUD|KillFeed")
	TArray<FString> KillFeedEntries;

	TObjectPtr<ABaseFPSCharacter> BoundCharacter;
	FDelegateHandle HealthChangedHandle;
	FDelegateHandle AmmoChangedHandle;
};

