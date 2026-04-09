#include "WeaponBase.h"

#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "GameFramework/Controller.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

AWeaponBase::AWeaponBase()
{
	bReplicates = true;
	PrimaryActorTick.bCanEverTick = false;

	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	RootComponent = WeaponMesh;
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AWeaponBase::BeginPlay()
{
	Super::BeginPlay();
}

void AWeaponBase::InitializeWeapon(ABaseFPSCharacter* InOwnerCharacter, EFPSWeaponSlot InSlot)
{
	OwnerCharacter = InOwnerCharacter;
	WeaponSlot = InSlot;
	SetOwner(InOwnerCharacter);
}

void AWeaponBase::OnEquipped(const FName& AttachSocketName)
{
	if (!OwnerCharacter)
	{
		return;
	}

	AttachToComponent(
		OwnerCharacter->GetMesh(),
		FAttachmentTransformRules::SnapToTargetNotIncludingScale,
		AttachSocketName);
}

void AWeaponBase::OnUnequipped()
{
	StopFire();
}

void AWeaponBase::StartFire()
{
	// Server authority only (we call this via ServerSetFiring)
	if (!HasAuthority())
	{
		return;
	}

	if (bIsFiring)
	{
		return;
	}

	bIsFiring = true;

	// Knife: treat as single action per press (no hold auto-fire)
	if (WeaponSlot == EFPSWeaponSlot::Melee)
	{
		FireOnce();
		bIsFiring = false;
		return;
	}

	FireOnce();

	if (bFullAuto)
	{
		GetWorldTimerManager().SetTimer(RefireTimerHandle, this, &AWeaponBase::FireOnce, RefireRate, true);
	}
}

void AWeaponBase::StopFire()
{
	if (!HasAuthority())
	{
		return;
	}

	bIsFiring = false;
	GetWorldTimerManager().ClearTimer(RefireTimerHandle);
}

void AWeaponBase::FireOnce()
{
	if (!HasAuthority())
	{
		return;
	}

	if (!OwnerCharacter)
	{
		return;
	}

	if (WeaponSlot == EFPSWeaponSlot::Melee)
	{
		PerformMeleeAttack();
		return;
	}

	FVector Start, End;
	if (!GetAimStartEnd(Start, End))
	{
		return;
	}

	FHitResult Hit;
	if (PerformHitscanTrace(Hit, Start, End) && Hit.GetActor())
	{
		ApplyPointDamageFromHit(Hit);
	}
}

bool AWeaponBase::GetAimStartEnd(FVector& OutStart, FVector& OutEnd) const
{
	if (!OwnerCharacter)
	{
		return false;
	}

	FVector EyeLocation;
	FRotator EyeRotation;
	OwnerCharacter->GetActorEyesViewPoint(EyeLocation, EyeRotation);

	OutStart = EyeLocation;
	OutEnd = EyeLocation + (EyeRotation.Vector() * Range);
	return true;
}

bool AWeaponBase::PerformHitscanTrace(FHitResult& OutHit, const FVector& Start, const FVector& End) const
{
	if (!GetWorld())
	{
		return false;
	}

	FCollisionQueryParams Params(SCENE_QUERY_STAT(WeaponTrace), /*bTraceComplex*/ true);
	Params.AddIgnoredActor(this);
	if (OwnerCharacter)
	{
		Params.AddIgnoredActor(OwnerCharacter);
	}

	return GetWorld()->LineTraceSingleByChannel(OutHit, Start, End, ECC_Visibility, Params);
}

void AWeaponBase::ApplyPointDamageFromHit(const FHitResult& Hit)
{
	AActor* HitActor = Hit.GetActor();
	if (!HitActor || !OwnerCharacter)
	{
		return;
	}

	APawn* InstigatorPawn = Cast<APawn>(OwnerCharacter);
	AController* InstigatorController = InstigatorPawn ? InstigatorPawn->GetController() : nullptr;

	const FVector ShotDirection = (Hit.TraceEnd - Hit.TraceStart).GetSafeNormal();
	UGameplayStatics::ApplyPointDamage(
		HitActor,
		Damage,
		ShotDirection,
		Hit,
		InstigatorController,
		this,
		UDamageType::StaticClass());
}

void AWeaponBase::PerformMeleeAttack()
{
	if (!GetWorld() || !OwnerCharacter)
	{
		return;
	}

	FVector EyeLocation;
	FRotator EyeRotation;
	OwnerCharacter->GetActorEyesViewPoint(EyeLocation, EyeRotation);

	const FVector Start = EyeLocation;
	const FVector End = EyeLocation + (EyeRotation.Vector() * MeleeRange);

	FCollisionShape Shape = FCollisionShape::MakeSphere(MeleeRadius);
	FCollisionQueryParams Params(SCENE_QUERY_STAT(MeleeTrace), /*bTraceComplex*/ false);
	Params.AddIgnoredActor(this);
	Params.AddIgnoredActor(OwnerCharacter);

	FHitResult Hit;
	const bool bHit = GetWorld()->SweepSingleByChannel(
		Hit,
		Start,
		End,
		FQuat::Identity,
		ECC_Pawn,
		Shape,
		Params);

	if (bHit && Hit.GetActor())
	{
		ApplyPointDamageFromHit(Hit);
	}
}
