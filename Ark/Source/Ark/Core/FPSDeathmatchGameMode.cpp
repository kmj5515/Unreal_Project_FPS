#include "FPSDeathmatchGameMode.h"

#include "FPSPlayerState.h"

AFPSDeathmatchGameMode::AFPSDeathmatchGameMode() = default;

void AFPSDeathmatchGameMode::ReportKill(
	AFPSPlayerState* KillerPlayerState,
	AFPSPlayerState* VictimPlayerState,
	AActor* DamageCauser)
{
	Super::ReportKill(KillerPlayerState, VictimPlayerState, DamageCauser);

	if (!KillerPlayerState || !VictimPlayerState || KillerPlayerState == VictimPlayerState)
	{
		return;
	}

	const int32 NextKills = FMath::FloorToInt(KillerPlayerState->GetScore()) + 1;
	KillerPlayerState->SetScore(static_cast<float>(NextKills));

	if (NextKills >= KillsToWin)
	{
		UE_LOG(
			LogTemp,
			Log,
			TEXT("[Deathmatch] Kill goal reached: %s (%d / %d)."),
			*KillerPlayerState->GetPlayerName(),
			NextKills,
			KillsToWin);
	}
}
