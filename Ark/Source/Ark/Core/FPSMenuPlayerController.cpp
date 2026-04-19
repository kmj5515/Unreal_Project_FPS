#include "FPSMenuPlayerController.h"

#include "Blueprint/UserWidget.h"

AFPSMenuPlayerController::AFPSMenuPlayerController()
{
	bShowMouseCursor = true;
	bEnableClickEvents = true;
	bEnableMouseOverEvents = true;
}

void AFPSMenuPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (!IsLocalController())
	{
		return;
	}

	FInputModeUIOnly InputMode;
	SetInputMode(InputMode);
	bShowMouseCursor = true;

	if (MainMenuWidgetClass && !MainMenuWidget)
	{
		MainMenuWidget = CreateWidget<UUserWidget>(this, MainMenuWidgetClass);
		if (MainMenuWidget)
		{
			MainMenuWidget->AddToViewport();
		}
	}
}

void AFPSMenuPlayerController::ShowLobby()
{
	if (MainMenuWidget)
	{
		MainMenuWidget->RemoveFromParent();
		MainMenuWidget = nullptr;
	}

	if (!LobbyWidgetClass || LobbyWidget)
	{
		return;
	}

	LobbyWidget = CreateWidget<UUserWidget>(this, LobbyWidgetClass);
	if (LobbyWidget)
	{
		LobbyWidget->AddToViewport();
	}
}

void AFPSMenuPlayerController::ShowMainMenu()
{
	if (LobbyWidget)
	{
		LobbyWidget->RemoveFromParent();
		LobbyWidget = nullptr;
	}

	if (!MainMenuWidgetClass || MainMenuWidget)
	{
		return;
	}

	MainMenuWidget = CreateWidget<UUserWidget>(this, MainMenuWidgetClass);
	if (MainMenuWidget)
	{
		MainMenuWidget->AddToViewport();
	}
}
