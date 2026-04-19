#include "FPSMainMenuWidget.h"

#include "../Core/FPSMenuPlayerController.h"
#include "Components/Button.h"
#include "Kismet/KismetSystemLibrary.h"

void UFPSMainMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (QuitGameButton)
	{
		QuitGameButton->OnClicked.AddDynamic(this, &UFPSMainMenuWidget::OnQuitClicked);
	}
	if (StartGameButton)
	{
		StartGameButton->OnClicked.AddDynamic(this, &UFPSMainMenuWidget::OnGameStartClicked);
	}
}

void UFPSMainMenuWidget::NativeDestruct()
{
	if (QuitGameButton)
	{
		QuitGameButton->OnClicked.RemoveDynamic(this, &UFPSMainMenuWidget::OnQuitClicked);
	}
	if (StartGameButton)
	{
		StartGameButton->OnClicked.RemoveDynamic(this, &UFPSMainMenuWidget::OnGameStartClicked);
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
