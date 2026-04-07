#include "FPSGameMode.h"

#include "../Characters/BaseFPSCharacter.h"
#include "FPSPlayerController.h"
#include "FPSPlayerState.h"

AFPSGameMode::AFPSGameMode()
{
	DefaultPawnClass = ABaseFPSCharacter::StaticClass();
	PlayerControllerClass = AFPSPlayerController::StaticClass();
	PlayerStateClass = AFPSPlayerState::StaticClass();
}
