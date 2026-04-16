#include "FPSGameMode.h"

#include "../Characters/BaseFPSCharacter.h"
#include "FPSPlayerController.h"
#include "FPSPlayerState.h"
#include "../UI/FPSGameHUD.h"

AFPSGameMode::AFPSGameMode()
{
	DefaultPawnClass = ABaseFPSCharacter::StaticClass();
	PlayerControllerClass = AFPSPlayerController::StaticClass();
	PlayerStateClass = AFPSPlayerState::StaticClass();
	HUDClass = AFPSGameHUD::StaticClass();
}
