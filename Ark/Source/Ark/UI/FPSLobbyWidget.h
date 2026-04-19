#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "FPSLobbyWidget.generated.h"

class UButton;
class UComboBoxString;

UCLASS()
class ARK_API UFPSLobbyWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

protected:
	UFUNCTION()
	void OnStartClicked();

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> Button_Start;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UComboBoxString> ComboBox_Mode;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Match")
	FString MatchMapPath = TEXT("/Game/Maps/TestMap");

	/** Appended to OpenLevel (e.g. listen server + game mode). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Match")
	FString MatchOptions = TEXT("listen?game=/Script/Ark.FPSDeathmatchGameMode");
};
