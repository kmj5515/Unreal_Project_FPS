#include "FPSPlayerController.h"

#include "../Characters/BaseFPSCharacter.h"
#include "../Components/FPSCombatComponent.h"
#include "../UI/FPSDebugToolWidget.h"
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

	if (DebugToolWidgetClass && !DebugToolWidget)
	{
		DebugToolWidget = CreateWidget<UFPSDebugToolWidget>(this, DebugToolWidgetClass);
		if (DebugToolWidget)
		{
			DebugToolWidget->BindToPlayerController(this);
			DebugToolWidget->AddToViewport(20);
			bDebugToolVisible = false;
			DebugToolWidget->SetVisibility(ESlateVisibility::Collapsed);
		}
	}

	BindHUDToCurrentPawn();
}

void AFPSPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (InputComponent)
	{
		InputComponent->BindKey(ToggleDebugToolKey, IE_Pressed, this, &AFPSPlayerController::HandleToggleDebugToolPressed);
	}
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
	if (HUDWidget)
	{
		HUDWidget->ShowKillStreakAnnouncement(KillStreakCount);
	}

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

void AFPSPlayerController::HandleToggleDebugToolPressed()
{
	ToggleDebugTool();
}

void AFPSPlayerController::ToggleDebugTool()
{
	if (!DebugToolWidgetClass)
	{
		return;
	}

	if (!DebugToolWidget)
	{
		DebugToolWidget = CreateWidget<UFPSDebugToolWidget>(this, DebugToolWidgetClass);
		if (!DebugToolWidget)
		{
			return;
		}

		DebugToolWidget->BindToPlayerController(this);
		DebugToolWidget->AddToViewport(20);
	}

	bDebugToolVisible = !bDebugToolVisible;
	DebugToolWidget->SetVisibility(bDebugToolVisible ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
}

void AFPSPlayerController::SetHitboxDebugEnabled(bool bEnabled)
{
	bHitboxDebugEnabled = bEnabled;
	ApplyHitboxDebugCommand();
}

void AFPSPlayerController::ApplyHitboxDebugCommand()
{
	const FString& Command = bHitboxDebugEnabled ? HitboxDebugOnCommand : HitboxDebugOffCommand;
	if (!Command.IsEmpty())
	{
		ConsoleCommand(Command, true);
	}
}

void AFPSPlayerController::SetTraceDebugEnabled(bool bEnabled)
{
	bTraceDebugEnabled = bEnabled;
	if (ABaseFPSCharacter* FPSChar = GetPawn<ABaseFPSCharacter>())
	{
		if (FPSChar->CombatComponent)
		{
			FPSChar->CombatComponent->SetTraceDebugEnabled(bTraceDebugEnabled);
		}
	}
}

void AFPSPlayerController::SetDpsMeasureEnabled(bool bEnabled)
{
	bDpsMeasuring = bEnabled;
	if (ABaseFPSCharacter* FPSChar = GetPawn<ABaseFPSCharacter>())
	{
		if (FPSChar->CombatComponent)
		{
			FPSChar->CombatComponent->SetDpsMeasureEnabled(bDpsMeasuring);
		}
	}
}

void AFPSPlayerController::SetInfiniteAmmoEnabled(bool bEnabled)
{
	bInfiniteAmmoEnabled = bEnabled;
	if (ABaseFPSCharacter* FPSChar = GetPawn<ABaseFPSCharacter>())
	{
		if (FPSChar->CombatComponent)
		{
			FPSChar->CombatComponent->SetInfiniteAmmoEnabled(bInfiniteAmmoEnabled);
		}
	}
}

void AFPSPlayerController::GetDpsStats(int32& OutShotCount, float& OutTotalDamage, float& OutElapsedSeconds, float& OutDps) const
{
	OutShotCount = 0;
	OutTotalDamage = 0.f;
	OutElapsedSeconds = 0.f;
	OutDps = 0.f;

	if (const ABaseFPSCharacter* FPSChar = GetPawn<ABaseFPSCharacter>())
	{
		if (FPSChar->CombatComponent)
		{
			FPSChar->CombatComponent->GetDpsStats(OutShotCount, OutTotalDamage, OutElapsedSeconds, OutDps);
		}
	}
}

void AFPSPlayerController::SetCurrentWeaponSpread(float NewSpreadDeg)
{
	if (ABaseFPSCharacter* FPSChar = GetPawn<ABaseFPSCharacter>())
	{
		if (FPSChar->CombatComponent)
		{
			FPSChar->CombatComponent->SetCurrentWeaponSpread(NewSpreadDeg);
		}
	}
}

float AFPSPlayerController::GetCurrentWeaponSpread() const
{
	if (const ABaseFPSCharacter* FPSChar = GetPawn<ABaseFPSCharacter>())
	{
		if (FPSChar->CombatComponent)
		{
			return FPSChar->CombatComponent->GetCurrentWeaponSpread();
		}
	}

	return 0.f;
}

void AFPSPlayerController::SetCurrentWeaponAmmoDebug(int32 NewAmmoInMagazine, int32 NewMagazineSize, int32 NewMaxCarryAmmo)
{
	if (ABaseFPSCharacter* FPSChar = GetPawn<ABaseFPSCharacter>())
	{
		if (FPSChar->CombatComponent)
		{
			FPSChar->CombatComponent->SetCurrentWeaponAmmoDebug(NewAmmoInMagazine, NewMagazineSize, NewMaxCarryAmmo);
		}
	}
}

void AFPSPlayerController::GetCurrentWeaponAmmoDebug(int32& OutAmmoInMagazine, int32& OutMagazineSize, int32& OutReserveAmmo, int32& OutMaxCarryAmmo) const
{
	OutAmmoInMagazine = 0;
	OutMagazineSize = 0;
	OutReserveAmmo = 0;
	OutMaxCarryAmmo = 0;

	if (const ABaseFPSCharacter* FPSChar = GetPawn<ABaseFPSCharacter>())
	{
		if (FPSChar->CombatComponent)
		{
			FPSChar->CombatComponent->GetCurrentWeaponAmmoDebug(OutAmmoInMagazine, OutMagazineSize, OutReserveAmmo, OutMaxCarryAmmo);
		}
	}
}