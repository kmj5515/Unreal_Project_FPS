#include "FPSProjectileBullet.h"

#include "../Characters/BaseFPSCharacter.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"

AFPSProjectileBullet::AFPSProjectileBullet()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));
	SetRootComponent(CollisionSphere);
	CollisionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionSphere->SetCollisionResponseToAllChannels(ECR_Block);
	CollisionSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->InitialSpeed = 12000.f;
	ProjectileMovement->MaxSpeed = 12000.f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->ProjectileGravityScale = 0.f;
}

void AFPSProjectileBullet::InitializeProjectile(float InDamage, float InInitialSpeed, ABaseFPSCharacter* InDamageInstigator)
{
	Damage = InDamage;
	DamageInstigator = InDamageInstigator;
	if (ProjectileMovement)
	{
		ProjectileMovement->InitialSpeed = InInitialSpeed;
		ProjectileMovement->MaxSpeed = InInitialSpeed;
		ProjectileMovement->Velocity = GetActorForwardVector() * InInitialSpeed;
	}
}

void AFPSProjectileBullet::BeginPlay()
{
	Super::BeginPlay();

	if (CollisionSphere)
	{
		CollisionSphere->OnComponentHit.AddDynamic(this, &AFPSProjectileBullet::OnProjectileHit);
	}

	if (TraceParticle)
	{
		TraceParticleComponent = UGameplayStatics::SpawnEmitterAttached(
			TraceParticle,
			RootComponent,
			NAME_None,
			FVector::ZeroVector,
			FRotator::ZeroRotator,
			EAttachLocation::KeepRelativeOffset,
			true);
	}

	SetLifeSpan(MaxLifeSeconds);
}

void AFPSProjectileBullet::OnProjectileHit(
	UPrimitiveComponent* HitComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	FVector NormalImpulse,
	const FHitResult& Hit)
{
	if (HasAuthority() && OtherActor && OtherActor != this && OtherActor != DamageInstigator)
	{
		APawn* InstigatorPawn = Cast<APawn>(DamageInstigator);
		AController* InstigatorController = InstigatorPawn ? InstigatorPawn->GetController() : nullptr;
		const FVector ShotDirection = GetActorForwardVector();

		UGameplayStatics::ApplyPointDamage(
			OtherActor,
			Damage,
			ShotDirection,
			Hit,
			InstigatorController,
			this,
			UDamageType::StaticClass());
	}

	Destroy();
}
