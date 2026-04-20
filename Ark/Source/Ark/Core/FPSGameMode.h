#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "FPSGameMode.generated.h"

class AFPSPlayerState;
class AActor;
class AFPSPlayerController;

UCLASS()
class ARK_API AFPSGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AFPSGameMode();
	virtual void ReportKill(AFPSPlayerState* KillerPlayerState, AFPSPlayerState* VictimPlayerState, AActor* DamageCauser, bool bWasHeadshot);

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Match|KillStreak", meta = (ClampMin = "1.0"))
	float MultiKillWindowSeconds = 10.0f;

private:
	struct FKillStreakData
	{
		int32 Count = 0;
		float LastKillTime = -1000.f;
	};

	TMap<TWeakObjectPtr<AFPSPlayerState>, FKillStreakData> KillStreakByPlayer;
	int32 RegisterKillAndGetStreak(AFPSPlayerState* KillerPlayerState);
	void NotifyKiller(AFPSPlayerState* KillerPlayerState, bool bWasHeadshot, int32 StreakCount);
};
