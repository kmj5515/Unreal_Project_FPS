#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "FPSPlayerController.generated.h"

class UInputMappingContext;
class UFPSHUDWidget;
class USoundBase;

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

protected:
	virtual void BeginPlay() override;
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnRep_Pawn() override;
	void BindHUDToCurrentPawn();

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

	UPROPERTY(EditDefaultsOnly, Category = "HUD")
	TSubclassOf<UFPSHUDWidget> HUDWidgetClass;

	UPROPERTY()
	TObjectPtr<UFPSHUDWidget> HUDWidget;

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
};
