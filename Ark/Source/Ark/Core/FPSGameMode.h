#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "FPSGameMode.generated.h"

class AFPSPlayerState;
class AActor;

UCLASS()
class ARK_API AFPSGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AFPSGameMode();
	void ReportKill(AFPSPlayerState* KillerPlayerState, AFPSPlayerState* VictimPlayerState, AActor* DamageCauser);
};
