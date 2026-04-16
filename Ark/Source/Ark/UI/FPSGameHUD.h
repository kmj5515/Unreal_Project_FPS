#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "FPSGameHUD.generated.h"

class UTexture2D;

USTRUCT(BlueprintType)
struct FFPSHUDPackage
{
	GENERATED_BODY()

public:
	TObjectPtr<UTexture2D> CrosshairsCenter = nullptr;
	TObjectPtr<UTexture2D> CrosshairsLeft = nullptr;
	TObjectPtr<UTexture2D> CrosshairsRight = nullptr;
	TObjectPtr<UTexture2D> CrosshairsTop = nullptr;
	TObjectPtr<UTexture2D> CrosshairsBottom = nullptr;
	float CrosshairSpread = 0.f;
	FLinearColor CrosshairsColor = FLinearColor::White;
};

UCLASS()
class ARK_API AFPSGameHUD : public AHUD
{
	GENERATED_BODY()

public:
	virtual void DrawHUD() override;
	void SetHUDPackage(const FFPSHUDPackage& InHUDPackage) { HUDPackage = InHUDPackage; }

private:
	void DrawCrosshair(UTexture2D* Texture, const FVector2D& ViewportCenter, const FVector2D& Spread, const FLinearColor& CrosshairColor);

	UPROPERTY(EditAnywhere, Category = "Crosshair")
	float CrosshairSpreadMax = 28.f;

	FFPSHUDPackage HUDPackage;
};
