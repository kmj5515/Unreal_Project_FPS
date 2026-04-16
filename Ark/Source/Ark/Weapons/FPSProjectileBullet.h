#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameFramework/Actor.h"
#include "FPSProjectileBullet.generated.h"

class UBoxComponent;
class UProjectileMovementComponent;
class ABaseFPSCharacter;
class UParticleSystem;
class UParticleSystemComponent;
class UGameplayEffect;

UCLASS()
class ARK_API AFPSProjectileBullet : public AActor
{
	GENERATED_BODY()

public:
	AFPSProjectileBullet();
	void InitializeProjectile(
		float InDamage,
		float InInitialSpeed,
		ABaseFPSCharacter* InDamageInstigator,
		TSubclassOf<UGameplayEffect> InDamageGameplayEffect,
		FGameplayTag InDamageSetByCallerTag);

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnProjectileHit(
		UPrimitiveComponent* HitComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		FVector NormalImpulse,
		const FHitResult& Hit);

	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_DebugProjectileImpact(const FVector_NetQuantize& ImpactPoint, bool bHitPawn);

	UPROPERTY(VisibleAnywhere, Category = "Projectile")
	TObjectPtr<UBoxComponent> CollisionBox;

	UPROPERTY(EditDefaultsOnly, Category = "Projectile|Collision", meta = (ClampMin = "0.1", Units = "cm"))
	FVector CollisionBoxExtent = FVector(2.5f, 2.5f, 2.5f);

	UPROPERTY(VisibleAnywhere, Category = "Projectile")
	TObjectPtr<UProjectileMovementComponent> ProjectileMovement;

	UPROPERTY(EditDefaultsOnly, Category = "Projectile")
	float MaxLifeSeconds = 3.f;

	UPROPERTY(EditDefaultsOnly, Category = "Projectile|FX")
	TObjectPtr<UParticleSystem> TraceParticle;

	UPROPERTY()
	TObjectPtr<UParticleSystemComponent> TraceParticleComponent;

	UPROPERTY(EditAnywhere, Category = "Projectile|Debug")
	bool bDebugDrawImpact = true;

	UPROPERTY(EditAnywhere, Category = "Projectile|Debug", meta = (ClampMin = "0.0"))
	float DebugDrawDuration = 1.5f;

	float Damage = 25.f;
	bool bDamageApplied = false;

	UPROPERTY()
	TSubclassOf<UGameplayEffect> DamageGameplayEffect;

	FGameplayTag DamageSetByCallerTag;

	UPROPERTY()
	TObjectPtr<ABaseFPSCharacter> DamageInstigator;
};
