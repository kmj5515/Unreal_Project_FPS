#include "FPSMainMenuGameMode.h"

#include "FPSMenuPlayerController.h"
#include "GameFramework/DefaultPawn.h"

AFPSMainMenuGameMode::AFPSMainMenuGameMode()
{
	PlayerControllerClass = AFPSMenuPlayerController::StaticClass();
	DefaultPawnClass = ADefaultPawn::StaticClass();
	HUDClass = nullptr;
}
