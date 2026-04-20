#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "InputCoreTypes.h"
#include "FPSPlayerController.generated.h"

class UInputMappingContext;
class UFPSHUDWidget;
class USoundBase;
class UFPSDebugToolWidget;

UCLASS()
class ARK_API AFPSPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AFPSPlayerController();

	UFUNCTION(Client, Reliable)
	void ClientReceiveKillLog(const FString& KillerName, const FString& VictimName, const FString& WeaponName);

	UFUNCTION(Client, Reliable)
	void ClientNotifyKillEvent(bool bWasHeadshot, int32 KillStreakCount);

	UFUNCTION(BlueprintCallable, Category = "Debug")
	void ToggleDebugTool();

	UFUNCTION(BlueprintCallable, Category = "Debug")
	void SetHitboxDebugEnabled(bool bEnabled);

	UFUNCTION(BlueprintCallable, Category = "Debug")
	void SetTraceDebugEnabled(bool bEnabled);

	UFUNCTION(BlueprintCallable, Category = "Debug")
	void SetDpsMeasureEnabled(bool bEnabled);

	UFUNCTION(BlueprintPure, Category = "Debug")
	bool IsHitboxDebugEnabled() const { return bHitboxDebugEnabled; }

	UFUNCTION(BlueprintPure, Category = "Debug")
	bool IsTraceDebugEnabled() const { return bTraceDebugEnabled; }

	UFUNCTION(BlueprintPure, Category = "Debug")
	bool IsDpsMeasuring() const { return bDpsMeasuring; }

	UFUNCTION(BlueprintCallable, Category = "Debug")
	void GetDpsStats(int32& OutShotCount, float& OutTotalDamage, float& OutElapsedSeconds, float& OutDps) const;

	UFUNCTION(BlueprintCallable, Category = "Debug|Weapon")
	void SetCurrentWeaponSpread(float NewSpreadDeg);

	UFUNCTION(BlueprintPure, Category = "Debug|Weapon")
	float GetCurrentWeaponSpread() const;

	UFUNCTION(BlueprintCallable, Category = "Debug|Weapon")
	void SetCurrentWeaponAmmoDebug(int32 NewAmmoInMagazine, int32 NewMagazineSize, int32 NewMaxCarryAmmo);

	UFUNCTION(BlueprintCallable, Category = "Debug|Weapon")
	void GetCurrentWeaponAmmoDebug(int32& OutAmmoInMagazine, int32& OutMagazineSize, int32& OutReserveAmmo, int32& OutMaxCarryAmmo) const;

protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnRep_Pawn() override;
	void BindHUDToCurrentPawn();
	void HandleToggleDebugToolPressed();
	void ApplyHitboxDebugCommand();

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

	UPROPERTY(EditDefaultsOnly, Category = "HUD")
	TSubclassOf<UFPSHUDWidget> HUDWidgetClass;

	UPROPERTY()
	TObjectPtr<UFPSHUDWidget> HUDWidget;

	UPROPERTY(EditDefaultsOnly, Category = "HUD")
	TSubclassOf<UFPSDebugToolWidget> DebugToolWidgetClass;

	UPROPERTY()
	TObjectPtr<UFPSDebugToolWidget> DebugToolWidget;

	UPROPERTY(EditDefaultsOnly, Category = "Debug")
	FKey ToggleDebugToolKey = EKeys::F10;

	UPROPERTY(EditDefaultsOnly, Category = "Debug")
	FString HitboxDebugOnCommand = TEXT("ShowFlag.Collision 1");

	UPROPERTY(EditDefaultsOnly, Category = "Debug")
	FString HitboxDebugOffCommand = TEXT("ShowFlag.Collision 0");

	UPROPERTY(EditDefaultsOnly, Category = "Audio|Kill")
	TObjectPtr<USoundBase> HeadshotConfirmSound;

	UPROPERTY(EditDefaultsOnly, Category = "Audio|Kill")
	TObjectPtr<USoundBase> DoubleKillSound;

	UPROPERTY(EditDefaultsOnly, Category = "Audio|Kill")
	TObjectPtr<USoundBase> MultiKillSound;

	UPROPERTY(EditDefaultsOnly, Category = "Audio|Kill")
	TObjectPtr<USoundBase> QuadraKillSound;

	UPROPERTY(EditDefaultsOnly, Category = "Audio|Kill")
	TObjectPtr<USoundBase> PentaKillSound;

	bool bDebugToolVisible = false;
	bool bHitboxDebugEnabled = false;
	bool bTraceDebugEnabled = false;
	bool bDpsMeasuring = false;
};
