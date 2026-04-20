#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TimerManager.h"
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

	UFUNCTION(BlueprintCallable, Category = "HUD|KillStreak")
	void ShowKillStreakAnnouncement(int32 KillStreakCount);

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
	void ClearKillStreakAnnouncement();
	void RefreshKillStreakText();
	bool ResolveKillStreakPresentation(int32 KillStreakCount, FString& OutText, int32& OutPriority, float& OutDuration) const;
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

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> TextBlock_KillStreak;

	UPROPERTY(EditDefaultsOnly, Category = "HUD|KillFeed", meta = (ClampMin = "1", ClampMax = "12"))
	int32 MaxKillFeedEntries = 5;

	UPROPERTY(BlueprintReadOnly, Category = "HUD|KillFeed")
	TArray<FString> KillFeedEntries;

	UPROPERTY(EditDefaultsOnly, Category = "HUD|KillStreak", meta = (ClampMin = "0.1"))
	float DoubleKillDuration = 1.8f;

	UPROPERTY(EditDefaultsOnly, Category = "HUD|KillStreak", meta = (ClampMin = "0.1"))
	float MultiKillDuration = 2.0f;

	UPROPERTY(EditDefaultsOnly, Category = "HUD|KillStreak", meta = (ClampMin = "0.1"))
	float QuadraKillDuration = 2.2f;

	UPROPERTY(EditDefaultsOnly, Category = "HUD|KillStreak", meta = (ClampMin = "0.1"))
	float PentaKillDuration = 2.4f;

	UPROPERTY(BlueprintReadOnly, Category = "HUD|KillStreak")
	FString CurrentKillStreakText;

	TObjectPtr<ABaseFPSCharacter> BoundCharacter;
	FDelegateHandle HealthChangedHandle;
	FDelegateHandle AmmoChangedHandle;
	FTimerHandle KillStreakClearTimerHandle;
	int32 ActiveKillStreakPriority = 0;
};

