#include "FPSGameHUD.h"

#include "Engine/Texture2D.h"
#include "Engine/Engine.h"

void AFPSGameHUD::DrawHUD()
{
	Super::DrawHUD();

	if (!GEngine || !GEngine->GameViewport)
	{
		return;
	}

	FVector2D ViewportSize;
	GEngine->GameViewport->GetViewportSize(ViewportSize);
	const FVector2D ViewportCenter(ViewportSize.X * 0.5f, ViewportSize.Y * 0.5f);
	const float SpreadScaled = CrosshairSpreadMax * HUDPackage.CrosshairSpread;

	if (HUDPackage.CrosshairsCenter)
	{
		DrawCrosshair(HUDPackage.CrosshairsCenter, ViewportCenter, FVector2D::ZeroVector, HUDPackage.CrosshairsColor);
	}
	if (HUDPackage.CrosshairsLeft)
	{
		DrawCrosshair(HUDPackage.CrosshairsLeft, ViewportCenter, FVector2D(-SpreadScaled, 0.f), HUDPackage.CrosshairsColor);
	}
	if (HUDPackage.CrosshairsRight)
	{
		DrawCrosshair(HUDPackage.CrosshairsRight, ViewportCenter, FVector2D(SpreadScaled, 0.f), HUDPackage.CrosshairsColor);
	}
	if (HUDPackage.CrosshairsTop)
	{
		DrawCrosshair(HUDPackage.CrosshairsTop, ViewportCenter, FVector2D(0.f, -SpreadScaled), HUDPackage.CrosshairsColor);
	}
	if (HUDPackage.CrosshairsBottom)
	{
		DrawCrosshair(HUDPackage.CrosshairsBottom, ViewportCenter, FVector2D(0.f, SpreadScaled), HUDPackage.CrosshairsColor);
	}
}

void AFPSGameHUD::DrawCrosshair(UTexture2D* Texture, const FVector2D& ViewportCenter, const FVector2D& Spread, const FLinearColor& CrosshairColor)
{
	if (!Texture)
	{
		return;
	}

	const float TextureWidth = static_cast<float>(Texture->GetSizeX());
	const float TextureHeight = static_cast<float>(Texture->GetSizeY());
	const FVector2D DrawPoint(
		ViewportCenter.X - (TextureWidth * 0.5f) + Spread.X,
		ViewportCenter.Y - (TextureHeight * 0.5f) + Spread.Y);

	DrawTexture(
		Texture,
		DrawPoint.X,
		DrawPoint.Y,
		TextureWidth,
		TextureHeight,
		0.f,
		0.f,
		1.f,
		1.f,
		CrosshairColor);
}
