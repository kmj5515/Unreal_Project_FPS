#include "FPSMainMenuWidget.h"

#include "../Core/FPSMenuPlayerController.h"
#include "Components/Button.h"
#include "Kismet/KismetSystemLibrary.h"

void UFPSMainMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (Button_Quit)
	{
		Button_Quit->OnClicked.AddDynamic(this, &UFPSMainMenuWidget::OnQuitClicked);
	}
	if (Button_GameStart)
	{
		Button_GameStart->OnClicked.AddDynamic(this, &UFPSMainMenuWidget::OnGameStartClicked);
	}
}

void UFPSMainMenuWidget::NativeDestruct()
{
	if (Button_Quit)
	{
		Button_Quit->OnClicked.RemoveDynamic(this, &UFPSMainMenuWidget::OnQuitClicked);
	}
	if (Button_GameStart)
	{
		Button_GameStart->OnClicked.RemoveDynamic(this, &UFPSMainMenuWidget::OnGameStartClicked);
	}
	Super::NativeDestruct();
}

void UFPSMainMenuWidget::OnQuitClicked()
{
	APlayerController* PC = GetOwningPlayer();
	UKismetSystemLibrary::QuitGame(this, PC, EQuitPreference::Quit, false);
}

void UFPSMainMenuWidget::OnGameStartClicked()
{
	if (AFPSMenuPlayerController* MenuPC = Cast<AFPSMenuPlayerController>(GetOwningPlayer()))
	{
		MenuPC->ShowLobby();
	}
}
