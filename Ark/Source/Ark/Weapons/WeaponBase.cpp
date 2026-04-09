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
#include "Logging/LogMacros.h"
#include "TimerManager.h"
#include "DrawDebugHelpers.h"
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

	UE_LOG(LogTemp, Log, TEXT("[WeaponFire] StartFire: %s (Slot=%d)"), *GetName(), static_cast<int32>(WeaponSlot));

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
		UE_LOG(LogTemp, Log, TEXT("[WeaponFire] Hit: %s -> %s"), *GetName(), *Hit.GetActor()->GetName());
		if (!TryApplyGasDamageFromHit(Hit))
		{
			ApplyPointDamageFromHit(Hit);
		}
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("[WeaponFire] Miss: %s"), *GetName());
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

	FHitResult VisibilityHit;
	FHitResult PawnHit;
	const bool bHitVisibility = GetWorld()->LineTraceSingleByChannel(VisibilityHit, Start, End, ECC_Visibility, Params);
	const bool bHitPawn = GetWorld()->LineTraceSingleByChannel(PawnHit, Start, End, ECC_Pawn, Params);

	bool bHit = false;
	if (bHitVisibility && bHitPawn)
	{
		const float VisibilityDistSq = FVector::DistSquared(Start, VisibilityHit.ImpactPoint);
		const float PawnDistSq = FVector::DistSquared(Start, PawnHit.ImpactPoint);
		OutHit = (PawnDistSq <= VisibilityDistSq) ? PawnHit : VisibilityHit;
		bHit = true;
	}
	else if (bHitPawn)
	{
		OutHit = PawnHit;
		bHit = true;
	}
	else if (bHitVisibility)
	{
		OutHit = VisibilityHit;
		bHit = true;
	}

	if (bDebugDrawTrace)
	{
		const FVector DebugEnd = bHit ? OutHit.ImpactPoint : End;
		const FColor TraceColor = bHit ? FColor::Green : FColor::Red;
		DrawDebugLine(GetWorld(), Start, DebugEnd, TraceColor, false, DebugDrawDuration, 0, 1.5f);
		DrawDebugSphere(GetWorld(), DebugEnd, 8.f, 12, TraceColor, false, DebugDrawDuration);
	}

	return bHit;
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

	if (bDebugDrawTrace)
	{
		const FVector DebugEnd = bHit ? Hit.ImpactPoint : End;
		const FColor TraceColor = bHit ? FColor::Green : FColor::Red;
		DrawDebugCapsule(
			GetWorld(),
			(Start + End) * 0.5f,
			FVector::Distance(Start, End) * 0.5f,
			MeleeRadius,
			FQuat::FindBetweenNormals(FVector::UpVector, (End - Start).GetSafeNormal()),
			TraceColor,
			false,
			DebugDrawDuration,
			0,
			1.2f);
		DrawDebugSphere(GetWorld(), DebugEnd, 10.f, 12, TraceColor, false, DebugDrawDuration);
	}

	if (bHit && Hit.GetActor())
	{
		UE_LOG(LogTemp, Log, TEXT("[WeaponMelee] Hit: %s -> %s"), *GetName(), *Hit.GetActor()->GetName());
		if (!TryApplyGasDamageFromHit(Hit))
		{
			ApplyPointDamageFromHit(Hit);
		}
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("[WeaponMelee] Miss: %s"), *GetName());
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

	if (!DamageSetByCallerTag.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("[WeaponDamage] DamageSetByCallerTag is not set on %s"), *GetName());
		return false;
	}

	SpecHandle.Data->SetSetByCallerMagnitude(DamageSetByCallerTag, Damage);

	SourceASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetASC);
	return true;
}
