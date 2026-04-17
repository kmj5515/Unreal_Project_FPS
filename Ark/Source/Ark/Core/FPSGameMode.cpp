#include "FPSGameMode.h"

#include "../Characters/BaseFPSCharacter.h"
#include "FPSPlayerController.h"
#include "FPSPlayerState.h"
#include "../UI/FPSGameHUD.h"
#include "../Weapons/WeaponBase.h"

AFPSGameMode::AFPSGameMode()
{
	DefaultPawnClass = ABaseFPSCharacter::StaticClass();
	PlayerControllerClass = AFPSPlayerController::StaticClass();
	PlayerStateClass = AFPSPlayerState::StaticClass();
	HUDClass = AFPSGameHUD::StaticClass();
}

void AFPSGameMode::ReportKill(AFPSPlayerState* KillerPlayerState, AFPSPlayerState* VictimPlayerState, AActor* DamageCauser)
{
	if (!VictimPlayerState)
	{
		return;
	}

	const FString KillerName = KillerPlayerState
		? (KillerPlayerState->GetPlayerName().IsEmpty() ? TEXT("Unknown") : KillerPlayerState->GetPlayerName())
		: TEXT("World");
	const FString VictimName = VictimPlayerState->GetPlayerName().IsEmpty() ? TEXT("Unknown") : VictimPlayerState->GetPlayerName();

	FString WeaponName = TEXT("Unknown");
	if (const AWeaponBase* Weapon = Cast<AWeaponBase>(DamageCauser))
	{
		WeaponName = Weapon->GetKillFeedWeaponName();
	}
	else if (DamageCauser)
	{
		WeaponName = DamageCauser->GetName();
	}

	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		if (AFPSPlayerController* FPSPlayerController = Cast<AFPSPlayerController>(It->Get()))
		{
			FPSPlayerController->ClientReceiveKillLog(KillerName, VictimName, WeaponName);
		}
	}
}
