#include "FPSLobbyWidget.h"

#include "Components/Button.h"
#include "Components/ComboBoxString.h"
#include "Kismet/GameplayStatics.h"

void UFPSLobbyWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (ComboBox_Mode && ComboBox_Mode->GetOptionCount() == 0)
	{
		ComboBox_Mode->AddOption(TEXT("FFA Deathmatch"));
		ComboBox_Mode->SetSelectedIndex(0);
	}

	if (Button_Start)
	{
		Button_Start->OnClicked.AddDynamic(this, &UFPSLobbyWidget::OnStartClicked);
	}
}

void UFPSLobbyWidget::NativeDestruct()
{
	if (Button_Start)
	{
		Button_Start->OnClicked.RemoveDynamic(this, &UFPSLobbyWidget::OnStartClicked);
	}
	Super::NativeDestruct();
}

void UFPSLobbyWidget::OnStartClicked()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	UGameplayStatics::OpenLevel(World, FName(*MatchMapPath), true, MatchOptions);
}
