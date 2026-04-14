#pragma once

#include "CoreMinimal.h"
#include "FPSWeaponTypes.generated.h"

UENUM(BlueprintType)
enum class EFPSWeaponSlot : uint8
{
	Primary = 0,
	Secondary,
	Melee
};
