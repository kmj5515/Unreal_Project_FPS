#include "WeaponBase.h"

#include "../Core/FPSPlayerState.h"
#include "../GAS/FPSGameplayTags.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "GameFramework/Controller.h"
#include "GameFramework/Pawn.h"
#include "GameplayEffect.h"
#include "AbilitySystemComponent.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "WeaponDataAsset.h"

AWeaponBase::AWeaponBase()
{
	bReplicates = true;
	PrimaryActorTick.bCanEverTick = false;
	DamageSetByCallerTag = FPSGameplayTags::Data_Damage;

	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	RootComponent = WeaponMesh;
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AWeaponBase::BeginPlay()
{
	Super::BeginPlay();
	ApplyWeaponDataFromAsset();
}

void AWeaponBase::ApplyWeaponDataFromAsset()
{
	if (!WeaponData)
	{
		return;
	}

	Damage = WeaponData->Damage;
	Range = WeaponData->Range;
	RefireRate = WeaponData->RefireRate;
	bFullAuto = WeaponData->bFullAuto;
	MeleeRange = WeaponData->MeleeRange;
	MeleeRadius = WeaponData->MeleeRadius;

	if (WeaponData->DamageGameplayEffect)
	{
		DamageGameplayEffect = WeaponData->DamageGameplayEffect;
	}
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
	if (!HasAuthority())
	{
		return;
	}

	if (bIsFiring)
	{
		return;
	}

	bIsFiring = true;

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
		if (!TryApplyGasDamageFromHit(Hit))
		{
			ApplyPointDamageFromHit(Hit);
		}
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
		if (!TryApplyGasDamageFromHit(Hit))
		{
			ApplyPointDamageFromHit(Hit);
		}
	}
}

bool AWeaponBase::TryApplyGasDamageFromHit(const FHitResult& Hit)
{
	if (!DamageGameplayEffect || !OwnerCharacter || !Hit.GetActor())
	{
		return false;
	}

	AFPSPlayerState* SourcePS = OwnerCharacter->GetPlayerState<AFPSPlayerState>();
	if (!SourcePS)
	{
		return false;
	}

	UAbilitySystemComponent* SourceASC = SourcePS->GetAbilitySystemComponent();
	if (!SourceASC)
	{
		return false;
	}

	APawn* HitPawn = Cast<APawn>(Hit.GetActor());
	if (!HitPawn)
	{
		return false;
	}

	AFPSPlayerState* TargetPS = HitPawn->GetPlayerState<AFPSPlayerState>();
	if (!TargetPS)
	{
		return false;
	}

	UAbilitySystemComponent* TargetASC = TargetPS->GetAbilitySystemComponent();
	if (!TargetASC)
	{
		return false;
	}

	FGameplayEffectContextHandle EffectContext = SourceASC->MakeEffectContext();
	EffectContext.AddSourceObject(this);
	EffectContext.AddHitResult(Hit);

	const FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(DamageGameplayEffect, 1.f, EffectContext);
	if (!SpecHandle.IsValid())
	{
		return false;
	}

	if (DamageSetByCallerTag.IsValid())
	{
		SpecHandle.Data->SetSetByCallerMagnitude(DamageSetByCallerTag, Damage);
	}

	SourceASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetASC);
	return true;
}
