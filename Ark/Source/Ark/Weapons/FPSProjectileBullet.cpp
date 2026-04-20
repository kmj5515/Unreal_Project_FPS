#include "FPSProjectileBullet.h"

#include "../Characters/BaseFPSCharacter.h"
#include "../Core/FPSPlayerState.h"
#include "Components/BoxComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"
#include "DrawDebugHelpers.h"

namespace
{
bool IsHeadshotBone_Projectile(const FName& BoneName)
{
	if (BoneName.IsNone())
	{
		return false;
	}

	const FString BoneLower = BoneName.ToString().ToLower();
	return BoneLower.Contains(TEXT("head")) || BoneLower.Contains(TEXT("neck"));
}
}

AFPSProjectileBullet::AFPSProjectileBullet()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	constexpr ECollisionChannel ProjectileObjectChannel = ECC_GameTraceChannel1;

	CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
	SetRootComponent(CollisionBox);
	CollisionBox->SetBoxExtent(CollisionBoxExtent);
	CollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionBox->SetCollisionObjectType(ProjectileObjectChannel);
	CollisionBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	CollisionBox->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	CollisionBox->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Ignore);
	CollisionBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
	CollisionBox->SetCollisionResponseToChannel(ProjectileObjectChannel, ECR_Ignore);
	CollisionBox->SetNotifyRigidBodyCollision(true);

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->InitialSpeed = 12000.f;
	ProjectileMovement->MaxSpeed = 12000.f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->ProjectileGravityScale = 0.f;
}

void AFPSProjectileBullet::InitializeProjectile(
	float InDamage,
	float InInitialSpeed,
	ABaseFPSCharacter* InDamageInstigator,
	TSubclassOf<UGameplayEffect> InDamageGameplayEffect,
	FGameplayTag InDamageSetByCallerTag)
{
	Damage = InDamage;
	DamageInstigator = InDamageInstigator;
	DamageGameplayEffect = InDamageGameplayEffect;
	DamageSetByCallerTag = InDamageSetByCallerTag;
	if (DamageInstigator)
	{
		CollisionBox->IgnoreActorWhenMoving(DamageInstigator, true);
	}
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

	if (CollisionBox)
	{
		CollisionBox->OnComponentHit.AddDynamic(this, &AFPSProjectileBullet::OnProjectileHit);
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
	if (bDamageApplied || !HasAuthority() || !OtherActor || OtherActor == this || OtherActor == DamageInstigator || OtherActor == GetOwner())
	{
		return;
	}
	bDamageApplied = true;

	if (bDebugDrawImpact)
	{
		const FVector ImpactPoint = Hit.ImpactPoint.IsNearlyZero() ? Hit.Location : Hit.ImpactPoint;
		const bool bHitPawn = Cast<APawn>(OtherActor) != nullptr;
		Multicast_DebugProjectileImpact(ImpactPoint, bHitPawn);
	}

	if (DamageGameplayEffect && DamageInstigator)
	{
		AFPSPlayerState* SourcePS = DamageInstigator->GetPlayerState<AFPSPlayerState>();
		APawn* HitPawn = Cast<APawn>(OtherActor);
		AFPSPlayerState* TargetPS = HitPawn ? HitPawn->GetPlayerState<AFPSPlayerState>() : nullptr;
		if (SourcePS && TargetPS)
		{
			UAbilitySystemComponent* SourceASC = SourcePS->GetAbilitySystemComponent();
			UAbilitySystemComponent* TargetASC = TargetPS->GetAbilitySystemComponent();
			if (SourceASC && TargetASC && DamageSetByCallerTag.IsValid())
			{
				FGameplayEffectContextHandle EffectContext = SourceASC->MakeEffectContext();
				EffectContext.AddSourceObject(this);
				EffectContext.AddHitResult(Hit);

				const FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(DamageGameplayEffect, 1.f, EffectContext);
				if (SpecHandle.IsValid())
				{
					const float DamageMultiplier = IsHeadshotBone_Projectile(Hit.BoneName) ? 2.0f : 1.0f;
					SpecHandle.Data->SetSetByCallerMagnitude(DamageSetByCallerTag, Damage * DamageMultiplier);
					SourceASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetASC);
				}
			}
		}
	}

	if (CollisionBox)
	{
		CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
	Destroy();
}

void AFPSProjectileBullet::Multicast_DebugProjectileImpact_Implementation(const FVector_NetQuantize& ImpactPoint, bool bHitPawn)
{
	if (GetNetMode() == NM_DedicatedServer || !bDebugDrawImpact || !GetWorld())
	{
		return;
	}

	const FVector DebugPoint = FVector(ImpactPoint);
	const FColor ImpactColor = bHitPawn ? FColor::Orange : FColor::Yellow;
	DrawDebugSphere(GetWorld(), DebugPoint, 18.f, 16, ImpactColor, false, DebugDrawDuration, 0, 2.0f);
	DrawDebugPoint(GetWorld(), DebugPoint, 12.f, FColor::Cyan, false, DebugDrawDuration, 0);
}
