#pragma once

#include "CoreMinimal.h"
#include "FPSGameMode.h"
#include "FPSDeathmatchGameMode.generated.h"

UCLASS()
class ARK_API AFPSDeathmatchGameMode : public AFPSGameMode
{
	GENERATED_BODY()

public:
	AFPSDeathmatchGameMode();

	virtual void ReportKill(AFPSPlayerState* KillerPlayerState, AFPSPlayerState* VictimPlayerState, AActor* DamageCauser, bool bWasHeadshot) override;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Match|Deathmatch", meta = (ClampMin = "1"))
	int32 KillsToWin = 20;
};
