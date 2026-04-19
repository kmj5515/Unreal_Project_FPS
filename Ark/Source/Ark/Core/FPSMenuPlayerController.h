#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "FPSMenuPlayerController.generated.h"

class UUserWidget;

UCLASS()
class ARK_API AFPSMenuPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AFPSMenuPlayerController();

	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, Category = "Menu")
	void ShowLobby();

	UFUNCTION(BlueprintCallable, Category = "Menu")
	void ShowMainMenu();

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Menu|UI")
	TSubclassOf<UUserWidget> MainMenuWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "Menu|UI")
	TSubclassOf<UUserWidget> LobbyWidgetClass;

	UPROPERTY(Transient)
	TObjectPtr<UUserWidget> MainMenuWidget;

	UPROPERTY(Transient)
	TObjectPtr<UUserWidget> LobbyWidget;
};
