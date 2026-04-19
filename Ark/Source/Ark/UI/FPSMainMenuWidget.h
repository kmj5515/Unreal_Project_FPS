#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "FPSMainMenuWidget.generated.h"

class UButton;

UCLASS()
class ARK_API UFPSMainMenuWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

protected:
	UFUNCTION()
	void OnQuitClicked();

	UFUNCTION()
	void OnGameStartClicked();

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> QuitGameButton;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> StartGameButton;
};
