#include "FPSPlayerController.h"

#include "../Characters/BaseFPSCharacter.h"
#include "../UI/FPSHUDWidget.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "InputMappingContext.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"

AFPSPlayerController::AFPSPlayerController()
{
	bReplicates = true;
}

void AFPSPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (!IsLocalController())
	{
		return;
	}

	if (ULocalPlayer* LocalPlayer = GetLocalPlayer())
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
			LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
		{
			if (DefaultMappingContext)
			{
				Subsystem->AddMappingContext(DefaultMappingContext, 0);
			}
		}
	}

	if (HUDWidgetClass && !HUDWidget)
	{
		HUDWidget = CreateWidget<UFPSHUDWidget>(this, HUDWidgetClass);
		if (HUDWidget)
		{
			HUDWidget->AddToViewport();
		}
	}

	BindHUDToCurrentPawn();
}

void AFPSPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	BindHUDToCurrentPawn();
}

void AFPSPlayerController::OnRep_Pawn()
{
	Super::OnRep_Pawn();
	BindHUDToCurrentPawn();
}

void AFPSPlayerController::BindHUDToCurrentPawn()
{
	if (!HUDWidget)
	{
		return;
	}

	HUDWidget->BindToCharacter(GetPawn<ABaseFPSCharacter>());
}

void AFPSPlayerController::ClientReceiveKillLog_Implementation(const FString& KillerName, const FString& VictimName, const FString& WeaponName)
{
	if (!HUDWidget)
	{
		return;
	}

	HUDWidget->AddKillLogEntry(KillerName, VictimName, WeaponName);
}

void AFPSPlayerController::ClientNotifyKillEvent_Implementation(bool bWasHeadshot, int32 KillStreakCount)
{
	if (bWasHeadshot && HeadshotConfirmSound)
	{
		UGameplayStatics::PlaySound2D(this, HeadshotConfirmSound);
	}

	USoundBase* StreakSound = nullptr;
	if (KillStreakCount >= 5)
	{
		StreakSound = PentaKillSound;
	}
	else if (KillStreakCount == 4)
	{
		StreakSound = QuadraKillSound;
	}
	else if (KillStreakCount == 3)
	{
		StreakSound = MultiKillSound;
	}
	else if (KillStreakCount == 2)
	{
		StreakSound = DoubleKillSound;
	}

	if (StreakSound)
	{
		UGameplayStatics::PlaySound2D(this, StreakSound);
	}
}