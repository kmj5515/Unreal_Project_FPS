#include "WeaponBase.h"

#include "../Characters/BaseFPSCharacter.h"
#include "../Core/FPSPlayerState.h"
#include "../GAS/FPSGameplayTags.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "GameFramework/Controller.h"
#include "GameFramework/Pawn.h"
#include "GameplayEffect.h"
#include "AbilitySystemComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Kismet/GameplayStatics.h"
#include "Logging/LogMacros.h"
#include "Net/UnrealNetwork.h"
#include "Sound/SoundBase.h"
#include "TimerManager.h"
#include "DrawDebugHelpers.h"
#include "Particles/ParticleSystem.h"
#include "WeaponDataAsset.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/StaticMesh.h"
#include "Components/SphereComponent.h"
#include "FPSProjectileBullet.h"

namespace
{
USkeletalMeshComponent* ResolveBestAttachMesh(ABaseFPSCharacter* InCharacter, const FName& DesiredSocket)
{
	if (!InCharacter)
	{
		return nullptr;
	}

	USkeletalMeshComponent* DefaultMesh = InCharacter->GetMesh();
	if (DefaultMesh && DefaultMesh->GetSkeletalMeshAsset())
	{
		return DefaultMesh;
	}

	TArray<USkeletalMeshComponent*> MeshComponents;
	InCharacter->GetComponents<USkeletalMeshComponent>(MeshComponents);

	USkeletalMeshComponent* FallbackWithAsset = nullptr;
	for (USkeletalMeshComponent* MeshComp : MeshComponents)
	{
		if (!MeshComp || !MeshComp->GetSkeletalMeshAsset())
		{
			continue;
		}

		if (DesiredSocket != NAME_None && MeshComp->DoesSocketExist(DesiredSocket))
		{
			return MeshComp;
		}

		if (!FallbackWithAsset)
		{
			FallbackWithAsset = MeshComp;
		}
	}

	return FallbackWithAsset;
}
}

AWeaponBase::AWeaponBase()
{
	bReplicates = true;
	SetReplicateMovement(true);
	PrimaryActorTick.bCanEverTick = false;
	DamageSetByCallerTag = FPSGameplayTags::Data_Damage;

	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	RootComponent = WeaponMesh;
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ProjectileClass = AFPSProjectileBullet::StaticClass();

	PickupSphere = CreateDefaultSubobject<USphereComponent>(TEXT("PickupSphere"));
	PickupSphere->SetupAttachment(RootComponent);
	PickupSphere->SetSphereRadius(120.f);
	PickupSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	PickupSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	PickupSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
}

FString AWeaponBase::GetKillFeedWeaponName() const
{
	if (!KillFeedWeaponName.IsEmpty())
	{
		return KillFeedWeaponName.ToString();
	}

	return GetClass() ? GetClass()->GetName() : TEXT("Weapon");
}

void AWeaponBase::BeginPlay()
{
	Super::BeginPlay();
	ApplyWeaponDataFromAsset();

	if (HasAuthority() && PickupSphere)
	{
		PickupSphere->OnComponentBeginOverlap.AddDynamic(this, &AWeaponBase::OnPickupSphereBeginOverlap);
		PickupSphere->OnComponentEndOverlap.AddDynamic(this, &AWeaponBase::OnPickupSphereEndOverlap);
	}

	if (HasAuthority() && !OwnerCharacter && !GetOwner())
	{
		SetDroppedState(true);
	}
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
	FireMode = WeaponData->bUseProjectileFire ? EFPSFireMode::Projectile : EFPSFireMode::HitScan;
	ProjectileInitialSpeed = WeaponData->ProjectileInitialSpeed;
	MeleeRange = WeaponData->MeleeRange;
	MeleeRadius = WeaponData->MeleeRadius;

	if (WeaponData->DamageGameplayEffect)
	{
		DamageGameplayEffect = WeaponData->DamageGameplayEffect;
	}

	if (WeaponData->MuzzleFlashParticle)
	{
		MuzzleFlashParticle = WeaponData->MuzzleFlashParticle;
	}

	if (WeaponData->ShellEjectStaticMesh)
	{
		ShellEjectStaticMesh = WeaponData->ShellEjectStaticMesh;
	}

	ShellLifeSpan = FMath::Max(0.f, WeaponData->ShellLifeSpan);
	ShellImpulseStrength = FMath::Max(0.f, WeaponData->ShellImpulseStrength);

	if (WeaponData->FireSound)
	{
		FireSound = WeaponData->FireSound;
	}

	if (WeaponData->ReloadMontage)
	{
		ReloadMontage = WeaponData->ReloadMontage;
	}

	MagazineSize = FMath::Max(1, WeaponData->MagazineSize);
	AmmoInMagazine = MagazineSize;
}

void AWeaponBase::InitializeWeapon(ABaseFPSCharacter* InOwnerCharacter, EFPSWeaponSlot InSlot)
{
	OwnerCharacter = InOwnerCharacter;
	WeaponSlot = InSlot;
	SetOwner(InOwnerCharacter);
}

void AWeaponBase::OnEquipped(const FName& AttachSocketName)
{
	if (AttachSocketName != NAME_None)
	{
		EquippedSocketName = AttachSocketName;
	}

	SetWeaponState(EFPSWeaponNetState::Equipped);

	if (OwnerCharacter)
	{
		OwnerCharacter->NotifyAmmoChangedValues(AmmoInMagazine, MagazineSize);
	}
}

void AWeaponBase::OnUnequipped()
{
	StopFire();
	bIsReloading = false;
	GetWorldTimerManager().ClearTimer(ReloadTimerHandle);
	GetWorldTimerManager().ClearTimer(SemiAutoFireGateTimerHandle);
	bIsFiring = false;
	if (HasAuthority())
	{
		Multicast_OnReloadFinished();
	}
	if (OwnerCharacter)
	{
		OwnerCharacter->NotifyReloadFinished();
	}
}

void AWeaponBase::SetDroppedState(bool bDropped)
{
	SetWeaponState(bDropped ? EFPSWeaponNetState::Dropped : EFPSWeaponNetState::Equipped);
}

void AWeaponBase::SetWeaponState(EFPSWeaponNetState NewState)
{
	ABaseFPSCharacter* PreviousOwnerCharacter = OwnerCharacter;
	WeaponState = NewState;
	if (HasAuthority())
	{
		if (WeaponState != EFPSWeaponNetState::Dropped && OwnerCharacter)
		{
			SetOwner(OwnerCharacter);
		}
	}
	ApplyWeaponState();
	if (HasAuthority() && WeaponState == EFPSWeaponNetState::Dropped)
	{
		LastDropCharacter = PreviousOwnerCharacter;
		LastDropWorldTimeSeconds = GetWorld() ? GetWorld()->GetTimeSeconds() : -1000.f;
		SetOwner(nullptr);
		OwnerCharacter = nullptr;
	}
}

void AWeaponBase::ApplyWeaponState()
{
	if (WeaponState == EFPSWeaponNetState::Dropped)
	{
		ApplyDroppedState();
	}
	else
	{
		ApplyEquippedState();
	}
}

void AWeaponBase::ApplyEquippedState()
{
	if (OwnerCharacter)
	{
		USkeletalMeshComponent* OwnerMesh = ResolveBestAttachMesh(OwnerCharacter, EquippedSocketName);
		if (!OwnerMesh)
		{
			return;
		}

		const bool bHasSocket = (EquippedSocketName != NAME_None && OwnerMesh->DoesSocketExist(EquippedSocketName));
		const FName AttachSocketNameToUse = bHasSocket ? EquippedSocketName : NAME_None;

		AttachToComponent(
			OwnerMesh,
			FAttachmentTransformRules::SnapToTargetNotIncludingScale,
			AttachSocketNameToUse);
	}

	WeaponMesh->SetSimulatePhysics(false);
	WeaponMesh->SetEnableGravity(false);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	if (PickupSphere)
	{
		PickupSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}

void AWeaponBase::ApplyDroppedState()
{
	DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	WeaponMesh->SetSimulatePhysics(true);
	WeaponMesh->SetEnableGravity(true);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	WeaponMesh->SetCollisionResponseToAllChannels(ECR_Block);
	WeaponMesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	if (HasAuthority())
	{
		APawn* ViewPawn = OwnerCharacter ? OwnerCharacter : Cast<APawn>(GetOwner());
		FVector ImpulseDir = ViewPawn
			? ViewPawn->GetControlRotation().Vector().GetSafeNormal()
			: GetActorForwardVector().GetSafeNormal();
		if (ImpulseDir.IsNearlyZero())
		{
			ImpulseDir = FVector::ForwardVector;
		}
		WeaponMesh->AddImpulse(ImpulseDir * DropImpulseStrength, NAME_None, true);
	}

	if (PickupSphere)
	{
		PickupSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		if (HasAuthority())
		{
			GetWorldTimerManager().ClearTimer(ReenablePickupSphereTimerHandle);
			if (SelfPickupBlockAfterDropSeconds > 0.f)
			{
				GetWorldTimerManager().SetTimer(
					ReenablePickupSphereTimerHandle,
					this,
					&AWeaponBase::EnablePickupSphereAfterDropBlock,
					SelfPickupBlockAfterDropSeconds,
					false);
			}
			else
			{
				EnablePickupSphereAfterDropBlock();
			}
		}
	}
}

void AWeaponBase::EnablePickupSphereAfterDropBlock()
{
	if (PickupSphere && HasAuthority())
	{
		PickupSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	}
}

void AWeaponBase::StartFire()
{
	if (!HasAuthority())
	{
		return;
	}

	if (bIsReloading)
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

	if (AmmoInMagazine <= 0)
	{
		StartReload();
		bIsFiring = false;
		return;
	}

	FireOnce();

	if (bFullAuto)
	{
		GetWorldTimerManager().SetTimer(RefireTimerHandle, this, &AWeaponBase::FireOnce, RefireRate, true);
	}
	else
	{
		// Semi-auto weapons also need a fire gate based on RefireRate.
		GetWorldTimerManager().SetTimer(SemiAutoFireGateTimerHandle, this, &AWeaponBase::ResetFireGate, RefireRate, false);
	}
}

void AWeaponBase::StopFire()
{
	if (!HasAuthority())
	{
		return;
	}

	if (bFullAuto)
	{
		bIsFiring = false;
		GetWorldTimerManager().ClearTimer(RefireTimerHandle);
	}
}

void AWeaponBase::StartReload()
{
	if (!HasAuthority() || bIsReloading || WeaponSlot == EFPSWeaponSlot::Melee)
	{
		return;
	}

	if (!ReloadMontage)
	{
		return;
	}

	if (!CanReload())
	{
		return;
	}

	bIsReloading = true;
	bIsFiring = false;
	GetWorldTimerManager().ClearTimer(RefireTimerHandle);
	GetWorldTimerManager().ClearTimer(SemiAutoFireGateTimerHandle);
	Multicast_PlayReloadMontage(ReloadMontage);
	if (OwnerCharacter)
	{
		OwnerCharacter->NotifyReloadStarted();
	}

	const float ActualReloadDuration = FMath::Max(0.f, ReloadMontage->GetPlayLength());

	if (ActualReloadDuration <= 0.f)
	{
		FinishReload();
		return;
	}

	GetWorldTimerManager().SetTimer(ReloadTimerHandle, this, &AWeaponBase::FinishReload, ActualReloadDuration, false);
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

	if (AmmoInMagazine <= 0)
	{
		StartReload();
		if (bFullAuto)
		{
			StopFire();
		}
		return;
	}

	FVector Start, End;
	if (!GetAimStartEnd(Start, End))
	{
		return;
	}

	Multicast_PlayMuzzleFlash();
	AmmoInMagazine = FMath::Max(0, AmmoInMagazine - 1);
	if (OwnerCharacter)
	{
		OwnerCharacter->NotifyAmmoChangedValues(AmmoInMagazine, MagazineSize);
	}

	FHitResult Hit;
	if (FireMode == EFPSFireMode::Projectile)
	{
		SpawnProjectile(Start, End);
		if (bDebugDrawTrace)
		{
			Multicast_DebugShotTrace(Start, End, false, End);
		}
	}
	else
	{
		const bool bHit = PerformHitscanTrace(Hit, Start, End);
		const FVector ImpactPoint = bHit
			? (Hit.ImpactPoint.IsNearlyZero() ? Hit.Location : Hit.ImpactPoint)
			: End;

		if (bDebugDrawTrace)
		{
			Multicast_DebugShotTrace(Start, End, bHit, ImpactPoint);
			if (bHit && Hit.GetActor())
			{
				Multicast_DebugHitImpact(ImpactPoint);
			}
		}

		if (bHit && Hit.GetActor())
		{
			if (!TryApplyGasDamageFromHit(Hit))
			{
				ApplyPointDamageFromHit(Hit);
			}
		}
	}
}

void AWeaponBase::Multicast_DebugHitImpact_Implementation(const FVector_NetQuantize& ImpactPoint)
{
	if (GetNetMode() == NM_DedicatedServer || !bDebugDrawTrace || !GetWorld())
	{
		return;
	}

	DrawDebugSphere(GetWorld(), FVector(ImpactPoint), 16.f, 16, FColor::Yellow, false, DebugDrawDuration, 0, 2.2f);
	DrawDebugPoint(GetWorld(), FVector(ImpactPoint), 12.f, FColor::Cyan, false, DebugDrawDuration, 0);
}

void AWeaponBase::Multicast_DebugShotTrace_Implementation(
	const FVector_NetQuantize& TraceStart,
	const FVector_NetQuantize& TraceEnd,
	bool bHit,
	const FVector_NetQuantize& ImpactPoint)
{
	if (GetNetMode() == NM_DedicatedServer || !bDebugDrawTrace || !GetWorld())
	{
		return;
	}

	const FVector Start = FVector(TraceStart);
	const FVector End = bHit ? FVector(ImpactPoint) : FVector(TraceEnd);
	const FColor TraceColor = bHit ? FColor::Green : FColor::Red;
	DrawDebugLine(GetWorld(), Start, End, TraceColor, false, DebugDrawDuration, 0, 1.6f);
}

void AWeaponBase::SpawnProjectile(const FVector& Start, const FVector& End)
{
	if (!GetWorld() || !ProjectileClass)
	{
		return;
	}

	const FVector SpawnLocation = (WeaponMesh && WeaponMesh->DoesSocketExist(MuzzleSocketName))
		? WeaponMesh->GetSocketLocation(MuzzleSocketName)
		: Start;
	const FVector ToTarget = (End - SpawnLocation).GetSafeNormal();
	if (ToTarget.IsNearlyZero())
	{
		return;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.Instigator = Cast<APawn>(OwnerCharacter);
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	if (AFPSProjectileBullet* Projectile = GetWorld()->SpawnActor<AFPSProjectileBullet>(
		ProjectileClass,
		SpawnLocation,
		ToTarget.Rotation(),
		SpawnParams))
	{
		Projectile->InitializeProjectile(
			Damage,
			ProjectileInitialSpeed,
			OwnerCharacter,
			DamageGameplayEffect,
			DamageSetByCallerTag);
	}
}

void AWeaponBase::FinishReload()
{
	if (!HasAuthority())
	{
		return;
	}

	bIsReloading = false;
	AmmoInMagazine = MagazineSize;
	if (OwnerCharacter)
	{
		OwnerCharacter->NotifyAmmoChangedValues(AmmoInMagazine, MagazineSize);
	}
	Multicast_OnReloadFinished();
	if (OwnerCharacter)
	{
		OwnerCharacter->NotifyReloadFinished();
	}
}

void AWeaponBase::ResetFireGate()
{
	bIsFiring = false;
}

bool AWeaponBase::CanReload() const
{
	if (WeaponSlot == EFPSWeaponSlot::Melee)
	{
		return false;
	}

	return AmmoInMagazine < MagazineSize;
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
	const FVector CameraAimDir = EyeRotation.Vector();
	const FVector CameraTraceEnd = EyeLocation + (CameraAimDir * Range);

	if (WeaponMesh && WeaponMesh->GetSkeletalMeshAsset() && MuzzleSocketName != NAME_None
		&& WeaponMesh->DoesSocketExist(MuzzleSocketName))
	{
		OutStart = WeaponMesh->GetSocketLocation(MuzzleSocketName);
	}
	else
	{
		OutStart = EyeLocation;
	}

	FVector FocalPoint = CameraTraceEnd;
	if (GetWorld())
	{
		FCollisionQueryParams CameraTraceParams(SCENE_QUERY_STAT(CameraAimTrace), true);
		CameraTraceParams.AddIgnoredActor(this);
		CameraTraceParams.AddIgnoredActor(OwnerCharacter);

		FHitResult VisibilityHit;
		FHitResult PawnHit;
		const bool bHitVisibility = GetWorld()->LineTraceSingleByChannel(VisibilityHit, EyeLocation, CameraTraceEnd, ECC_Visibility, CameraTraceParams);
		const bool bHitPawn = GetWorld()->LineTraceSingleByChannel(PawnHit, EyeLocation, CameraTraceEnd, ECC_Pawn, CameraTraceParams);

		if (bHitVisibility && bHitPawn)
		{
			const float VisibilityDistSq = FVector::DistSquared(EyeLocation, VisibilityHit.ImpactPoint);
			const float PawnDistSq = FVector::DistSquared(EyeLocation, PawnHit.ImpactPoint);
			const FHitResult& SelectedHit = (PawnDistSq <= VisibilityDistSq) ? PawnHit : VisibilityHit;
			FocalPoint = SelectedHit.ImpactPoint;
		}
		else if (bHitPawn)
		{
			FocalPoint = PawnHit.ImpactPoint;
		}
		else if (bHitVisibility)
		{
			FocalPoint = VisibilityHit.ImpactPoint;
		}
	}

	FVector AimDir = (FocalPoint - OutStart).GetSafeNormal();
	if (AimDir.IsNearlyZero())
	{
		AimDir = CameraAimDir;
	}

	if (bUseBulletSpread)
	{
		const float CrosshairSpreadFactor = FMath::Max(0.f, OwnerCharacter->GetCrosshairSpread());
		const float SpreadHalfAngleDeg = BulletSpreadPerCrosshairDeg * CrosshairSpreadFactor;
		if (SpreadHalfAngleDeg > KINDA_SMALL_NUMBER)
		{
			AimDir = FMath::VRandCone(AimDir, FMath::DegreesToRadians(SpreadHalfAngleDeg));
		}
	}

	OutEnd = OutStart + (AimDir * Range);
	return true;
}

void AWeaponBase::Multicast_PlayMuzzleFlash_Implementation()
{
	if (GetNetMode() == NM_DedicatedServer)
	{
		return;
	}

	if (OwnerCharacter && OwnerCharacter->IsLocallyControlled())
	{
		OwnerCharacter->NotifyShotFired();
	}

	if (!WeaponMesh)
	{
		return;
	}

	const bool bHasMuzzleSocket = (MuzzleSocketName != NAME_None && WeaponMesh->DoesSocketExist(MuzzleSocketName));
	const bool bHasAmmoEjectSocket = (AmmoEjectSocketName != NAME_None && WeaponMesh->DoesSocketExist(AmmoEjectSocketName));

	if (MuzzleFlashParticle && bHasMuzzleSocket)
	{
		UGameplayStatics::SpawnEmitterAttached(
			MuzzleFlashParticle,
			WeaponMesh,
			MuzzleSocketName,
			FVector::ZeroVector,
			FRotator::ZeroRotator,
			FVector(MuzzleFlashScale),
			EAttachLocation::SnapToTarget,
			true);
	}

	if (ShellEjectStaticMesh && bHasAmmoEjectSocket)
	{
		const FTransform ShellSpawnTransform = WeaponMesh->GetSocketTransform(AmmoEjectSocketName, ERelativeTransformSpace::RTS_World);
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		if (AStaticMeshActor* ShellActor = GetWorld()->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), ShellSpawnTransform, SpawnParams))
		{
			if (UStaticMeshComponent* ShellMeshComp = ShellActor->GetStaticMeshComponent())
			{
				ShellMeshComp->SetMobility(EComponentMobility::Movable);
				ShellMeshComp->SetStaticMesh(ShellEjectStaticMesh);
				ShellMeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
				ShellMeshComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
				ShellMeshComp->SetSimulatePhysics(true);
				const FVector EjectDir = ShellSpawnTransform.GetRotation().GetRightVector().GetSafeNormal();
				ShellMeshComp->AddImpulse(EjectDir * ShellImpulseStrength, NAME_None, true);
			}

			if (ShellLifeSpan > 0.f)
			{
				ShellActor->SetLifeSpan(ShellLifeSpan);
			}
		}
	}

	if (FireSound && bHasMuzzleSocket)
	{
		UGameplayStatics::SpawnSoundAttached(
			FireSound,
			WeaponMesh,
			MuzzleSocketName,
			FVector::ZeroVector,
			EAttachLocation::SnapToTarget);
	}

}

void AWeaponBase::Multicast_PlayReloadMontage_Implementation(UAnimMontage* MontageToPlay)
{
	// Use RPC arguments for montages: ReloadMontage is not replicated; clients can have a null member even when
	// the server has applied WeaponData, so relying on the UObject sent with the multicast fixes local playback.
	if (GetNetMode() == NM_DedicatedServer || !OwnerCharacter || !MontageToPlay)
	{
		return;
	}

	USkeletalMeshComponent* OwnerMesh = OwnerCharacter->GetMesh();
	if (!OwnerMesh)
	{
		return;
	}

	UAnimInstance* OwnerAnim = OwnerMesh->GetAnimInstance();
	if (!OwnerAnim)
	{
		return;
	}

	OwnerAnim->Montage_Play(MontageToPlay, 1.f);
}

void AWeaponBase::Multicast_OnReloadFinished_Implementation()
{
	if (GetNetMode() == NM_DedicatedServer || !OwnerCharacter || !OwnerCharacter->IsLocallyControlled())
	{
		return;
	}

	USkeletalMeshComponent* OwnerMesh = OwnerCharacter->GetMesh();
	if (!OwnerMesh)
	{
		return;
	}
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

	if (!DamageSetByCallerTag.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("[WeaponDamage] DamageSetByCallerTag is not set on %s"), *GetName());
		return false;
	}

	const float DamageMultiplier = (Hit.BoneName == FName(TEXT("head"))) ? 2.0f : 1.0f;
	SpecHandle.Data->SetSetByCallerMagnitude(DamageSetByCallerTag, Damage * DamageMultiplier);

	SourceASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetASC);
	return true;
}

void AWeaponBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AWeaponBase, OwnerCharacter);
	DOREPLIFETIME(AWeaponBase, AmmoInMagazine);
	DOREPLIFETIME(AWeaponBase, WeaponState);
	DOREPLIFETIME(AWeaponBase, EquippedSocketName);
}

void AWeaponBase::OnRep_AmmoInMagazine()
{
	if (OwnerCharacter)
	{
		OwnerCharacter->NotifyAmmoChangedValues(AmmoInMagazine, MagazineSize);
	}
}

void AWeaponBase::OnRep_OwnerCharacter()
{
	if (OwnerCharacter)
	{
		OwnerCharacter->NotifyAmmoChangedValues(AmmoInMagazine, MagazineSize);
	}
	ApplyWeaponState();
}

void AWeaponBase::OnRep_WeaponState()
{
	ApplyWeaponState();
}

void AWeaponBase::OnPickupSphereBeginOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	ABaseFPSCharacter* OverlapCharacter = Cast<ABaseFPSCharacter>(OtherActor);
	if (!OverlapCharacter || WeaponState != EFPSWeaponNetState::Dropped)
	{
		return;
	}

	if (OverlapCharacter->HasAnyFlags(RF_ClassDefaultObject | RF_ArchetypeObject))
	{
		return;
	}

	if (HasAuthority() && LastDropCharacter == OverlapCharacter && GetWorld())
	{
		const float SinceDrop = GetWorld()->GetTimeSeconds() - LastDropWorldTimeSeconds;
		if (SinceDrop >= 0.f && SinceDrop < SelfPickupBlockAfterDropSeconds)
		{
			return;
		}
	}

	OverlapCharacter->SetOverlappingWeapon(this);

	if (UFPSCombatComponent* RuntimeCombatComponent = OverlapCharacter->FindComponentByClass<UFPSCombatComponent>())
	{
		RuntimeCombatComponent->TryAutoPickupWeaponFromOverlap(this);
	}
}

void AWeaponBase::OnPickupSphereEndOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex)
{
	ABaseFPSCharacter* OverlapCharacter = Cast<ABaseFPSCharacter>(OtherActor);
	if (!OverlapCharacter)
	{
		return;
	}

	if (OverlapCharacter->GetOverlappingWeapon() == this)
	{
		OverlapCharacter->SetOverlappingWeapon(nullptr);
	}
}
