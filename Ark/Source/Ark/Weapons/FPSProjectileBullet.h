#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FPSProjectileBullet.generated.h"

class USphereComponent;
class UProjectileMovementComponent;
class ABaseFPSCharacter;
class UParticleSystem;
class UParticleSystemComponent;

UCLASS()
class ARK_API AFPSProjectileBullet : public AActor
{
	GENERATED_BODY()

public:
	AFPSProjectileBullet();
	void InitializeProjectile(float InDamage, float InInitialSpeed, ABaseFPSCharacter* InDamageInstigator);

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnProjectileHit(
		UPrimitiveComponent* HitComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		FVector NormalImpulse,
		const FHitResult& Hit);

	UPROPERTY(VisibleAnywhere, Category = "Projectile")
	TObjectPtr<USphereComponent> CollisionSphere;

	UPROPERTY(VisibleAnywhere, Category = "Projectile")
	TObjectPtr<UProjectileMovementComponent> ProjectileMovement;

	UPROPERTY(EditDefaultsOnly, Category = "Projectile")
	float MaxLifeSeconds = 3.f;

	UPROPERTY(EditDefaultsOnly, Category = "Projectile|FX")
	TObjectPtr<UParticleSystem> TraceParticle;

	UPROPERTY()
	TObjectPtr<UParticleSystemComponent> TraceParticleComponent;

	float Damage = 25.f;

	UPROPERTY()
	TObjectPtr<ABaseFPSCharacter> DamageInstigator;
};
