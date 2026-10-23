// Copyright Epic Games, Inc. All Rights Reserved.

#include "Gameplay/TDZombieCharacter.h"

#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Animation/AnimBlueprintGeneratedClass.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AIController.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "TimerManager.h"
#include "UObject/ConstructorHelpers.h"
#include "extractionFPSGameMode.h"

namespace
{
	UMaterialInstanceDynamic* MakeFlatColor(UMaterialInterface* BaseMaterial, UObject* Outer, const FLinearColor& Color)
	{
		if (!BaseMaterial)
		{
			return nullptr;
		}

		UMaterialInstanceDynamic* Material = UMaterialInstanceDynamic::Create(BaseMaterial, Outer);
		if (Material)
		{
			// The engine's basic shape material exposes a Color vector parameter.
			Material->SetVectorParameterValue(TEXT("Color"), Color);
			Material->SetVectorParameterValue(TEXT("BaseColor"), Color);
		}
		return Material;
	}

	void TintComponent(UStaticMeshComponent* Component, const FLinearColor& Color)
	{
		if (Component)
		{
			if (UMaterialInstanceDynamic* Material = Cast<UMaterialInstanceDynamic>(Component->GetMaterial(0)))
			{
				Material->SetVectorParameterValue(TEXT("Color"), Color);
				Material->SetVectorParameterValue(TEXT("BaseColor"), Color);
			}
		}
	}
}

ATDZombieCharacter::ATDZombieCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	GetCapsuleComponent()->InitCapsuleSize(40.0f, 92.0f);
	AIControllerClass = AAIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::Spawned;

	static ConstructorHelpers::FObjectFinder<USkeletalMesh> ZombieMesh(
		TEXT("/Game/Characters/Enemies/YellowZombie/SK_SZombie_Variant_1.SK_SZombie_Variant_1"));
	if (ZombieMesh.Succeeded())
	{
		GetMesh()->SetSkeletalMesh(ZombieMesh.Object);
		GetMesh()->SetRelativeLocationAndRotation(FVector(0.0f, 0.0f, -92.0f), FRotator(0.0f, -90.0f, 0.0f));
	}

	static ConstructorHelpers::FClassFinder<UAnimInstance> ZombieAnim(
		TEXT("/Game/Characters/Mannequins/Anims/Unarmed/ABP_Unarmed"));
	if (ZombieAnim.Succeeded())
	{
		const UAnimBlueprintGeneratedClass* AnimationClass = Cast<UAnimBlueprintGeneratedClass>(ZombieAnim.Class);
		USkeletalMesh* CurrentZombieMesh = GetMesh()->GetSkeletalMeshAsset();
		// The imported art has its own rig. Only use the mannequin animation
		// blueprint if its skeleton actually matches; otherwise leave it in its
		// imported reference pose until a retargeted animation set is added.
		if (AnimationClass && CurrentZombieMesh
			&& AnimationClass->GetTargetSkeleton() == CurrentZombieMesh->GetSkeleton())
		{
			GetMesh()->SetAnimInstanceClass(ZombieAnim.Class);
		}
	}

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> ShapeMaterial(
		TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> ZombieMaterial(
		TEXT("/Game/Maps/Materials/M_Zombie_StylizedYellow_Runtime.M_Zombie_StylizedYellow_Runtime"));

	FName TorsoBone(TEXT("spine_03"));
	if (GetMesh()->GetBoneIndex(TorsoBone) == INDEX_NONE)
	{
		// Imported character rigs use different bone naming conventions. Choose
		// the last spine bone present so attached graphic gear follows the chest.
		for (int32 BoneIndex = 0; BoneIndex < GetMesh()->GetNumBones(); ++BoneIndex)
		{
			const FName BoneName = GetMesh()->GetBoneName(BoneIndex);
			if (BoneName.ToString().Contains(TEXT("spine"), ESearchCase::IgnoreCase))
			{
				TorsoBone = BoneName;
			}
		}
		if (GetMesh()->GetBoneIndex(TorsoBone) == INDEX_NONE && GetMesh()->GetNumBones() > 0)
		{
			TorsoBone = GetMesh()->GetBoneName(0);
		}
	}

	if (CubeMesh.Succeeded() && ShapeMaterial.Succeeded())
	{
		ChestHarness = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Graphic Chest Harness"));
		ChestHarness->SetupAttachment(GetMesh(), TorsoBone);
		ChestHarness->SetStaticMesh(CubeMesh.Object);
		ChestHarness->SetRelativeLocation(FVector(0.0f, 0.0f, 4.0f));
		ChestHarness->SetRelativeScale3D(FVector(0.31f, 0.25f, 0.28f));
		ChestHarness->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		ChestHarness->SetMaterial(0, MakeFlatColor(ShapeMaterial.Object, this, FLinearColor(0.035f, 0.045f, 0.07f)));

		// Salvaged field gear gives the yellow silhouettes a stronger, more
		// characterful read from the overhead camera without recoloring the skin.
		SalvagePack = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Graphic Salvage Pack"));
		SalvagePack->SetupAttachment(GetMesh(), TorsoBone);
		SalvagePack->SetStaticMesh(CubeMesh.Object);
		SalvagePack->SetRelativeLocation(FVector(-17.0f, -17.0f, 5.0f));
		SalvagePack->SetRelativeRotation(FRotator(0.0f, 0.0f, 8.0f));
		SalvagePack->SetRelativeScale3D(FVector(0.24f, 0.31f, 0.32f));
		SalvagePack->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		SalvagePack->SetMaterial(0, MakeFlatColor(ShapeMaterial.Object, this, FLinearColor(0.05f, 0.30f, 0.33f)));

		UtilityBelt = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Graphic Utility Belt"));
		UtilityBelt->SetupAttachment(GetMesh(), TorsoBone);
		UtilityBelt->SetStaticMesh(CubeMesh.Object);
		UtilityBelt->SetRelativeLocation(FVector(0.0f, 0.0f, -20.0f));
		UtilityBelt->SetRelativeScale3D(FVector(0.36f, 0.28f, 0.09f));
		UtilityBelt->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		UtilityBelt->SetMaterial(0, MakeFlatColor(ShapeMaterial.Object, this, FLinearColor(0.17f, 0.11f, 0.07f)));

		ChestStrap = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Graphic Salvage Strap"));
		ChestStrap->SetupAttachment(GetMesh(), TorsoBone);
		ChestStrap->SetStaticMesh(CubeMesh.Object);
		ChestStrap->SetRelativeLocation(FVector(0.0f, 0.0f, 4.0f));
		ChestStrap->SetRelativeRotation(FRotator(0.0f, 0.0f, -34.0f));
		ChestStrap->SetRelativeScale3D(FVector(0.16f, 0.34f, 0.30f));
		ChestStrap->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		ChestStrap->SetMaterial(0, MakeFlatColor(ShapeMaterial.Object, this, FLinearColor(0.92f, 0.36f, 0.035f)));

		WarningTag = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Graphic Hazard Tag"));
		WarningTag->SetupAttachment(ChestHarness);
		WarningTag->SetStaticMesh(CubeMesh.Object);
		WarningTag->SetRelativeLocation(FVector(0.0f, 0.0f, -0.52f));
		WarningTag->SetRelativeScale3D(FVector(0.16f, 0.38f, 0.10f));
		WarningTag->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		WarningTag->SetMaterial(0, MakeFlatColor(ShapeMaterial.Object, this, FLinearColor(1.0f, 0.70f, 0.05f)));

		FName HeadBone = TorsoBone;
		for (int32 BoneIndex = 0; BoneIndex < GetMesh()->GetNumBones(); ++BoneIndex)
		{
			const FName BoneName = GetMesh()->GetBoneName(BoneIndex);
			if (BoneName.ToString().Contains(TEXT("head"), ESearchCase::IgnoreCase))
			{
				HeadBone = BoneName;
				break;
			}
		}
		HeadLamp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Graphic Amber Head Lamp"));
		HeadLamp->SetupAttachment(GetMesh(), HeadBone);
		HeadLamp->SetStaticMesh(SphereMesh.Succeeded() ? SphereMesh.Object : CubeMesh.Object);
		HeadLamp->SetRelativeLocation(FVector(8.0f, 0.0f, 13.0f));
		HeadLamp->SetRelativeScale3D(FVector(0.10f, 0.12f, 0.09f));
		HeadLamp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		HeadLamp->SetMaterial(0, MakeFlatColor(ShapeMaterial.Object, this, FLinearColor(1.0f, 0.72f, 0.08f)));

		HazardStripe = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Graphic Hazard Stripe"));
		HazardStripe->SetupAttachment(ChestHarness);
		HazardStripe->SetStaticMesh(CubeMesh.Object);
		HazardStripe->SetRelativeLocation(FVector(0.0f, 0.0f, 0.7f));
		HazardStripe->SetRelativeRotation(FRotator(0.0f, 0.0f, 18.0f));
		HazardStripe->SetRelativeScale3D(FVector(0.25f, 1.03f, 0.08f));
		HazardStripe->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		HazardStripe->SetMaterial(0, MakeFlatColor(ShapeMaterial.Object, this, FLinearColor(0.02f, 0.9f, 1.0f)));

		if (SphereMesh.Succeeded())
		{
			ShoulderArmor = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Graphic Shoulder Armor"));
			ShoulderArmor->SetupAttachment(GetMesh(), TorsoBone);
			ShoulderArmor->SetStaticMesh(SphereMesh.Object);
			ShoulderArmor->SetRelativeLocation(FVector(0.0f, 0.0f, 22.0f));
			ShoulderArmor->SetRelativeScale3D(FVector(0.23f, 0.39f, 0.16f));
			ShoulderArmor->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			ShoulderArmor->SetMaterial(0, MakeFlatColor(ShapeMaterial.Object, this, FLinearColor(1.0f, 0.12f, 0.025f)));
		}

		// BasicShapeMaterial is static-mesh-only and silently falls back on skeletal meshes.
		if (ZombieMaterial.Succeeded())
		{
			for (int32 MaterialIndex = 0; MaterialIndex < GetMesh()->GetNumMaterials(); ++MaterialIndex)
			{
				GetMesh()->SetMaterial(MaterialIndex, ZombieMaterial.Object);
			}
		}
	}

	// The character capsule provides movement and hit collision; the skeletal
	// mesh does not need a second physics body for this top-down defense game.
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetCharacterMovement()->MaxWalkSpeed = MoveSpeed;
	GetCharacterMovement()->bOrientRotationToMovement = true;
	bUseControllerRotationYaw = false;
}

void ATDZombieCharacter::BeginPlay()
{
	Super::BeginPlay();
	GetCharacterMovement()->MaxWalkSpeed = MoveSpeed;
	if (Archetype != ETDZombieArchetype::Exploder)
	{
		GetWorldTimerManager().SetTimer(AttackTimer, this, &ATDZombieCharacter::AttackOutpost, AttackInterval, true, AttackInterval);
	}
}

void ATDZombieCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (bIsDead || OutpostTarget.IsNearlyZero())
	{
		return;
	}

	const FVector ToTarget = OutpostTarget - GetActorLocation();
	if (ToTarget.Size2D() > AttackRange * 0.8f)
	{
		AddMovementInput(ToTarget.GetSafeNormal2D(), 1.0f, true);
	}
	else
	{
		GetCharacterMovement()->StopMovementImmediately();
		if (Archetype == ETDZombieArchetype::Exploder && !bExploderFuseStarted)
		{
			bExploderFuseStarted = true;
			GetWorldTimerManager().SetTimer(ExploderFuseTimer, this, &ATDZombieCharacter::ExploderFuseExpired, 3.0f, false);
		}
	}
}

void ATDZombieCharacter::SetOutpostTarget(const FVector& TargetLocation)
{
	OutpostTarget = TargetLocation;
}

void ATDZombieCharacter::SetArchetype(ETDZombieArchetype NewArchetype, int32 WaveNumber)
{
	Archetype = NewArchetype;
	const float WaveHealthScale = 1.0f + FMath::Max(0, WaveNumber - 1) * 0.18f;
	AttackInterval = 2.0f;
	OutpostDamage = 5.0f;
	ExplosionDamage = 150.0f;
	GetMesh()->SetRelativeScale3D(FVector(1.0f));
	if (SalvagePack) { SalvagePack->SetRelativeScale3D(FVector(0.24f, 0.31f, 0.32f)); }
	if (ShoulderArmor) { ShoulderArmor->SetRelativeScale3D(FVector(0.23f, 0.39f, 0.16f)); }
	if (ChestHarness) { ChestHarness->SetRelativeScale3D(FVector(0.31f, 0.25f, 0.28f)); }
	if (UtilityBelt) { UtilityBelt->SetRelativeScale3D(FVector(0.36f, 0.28f, 0.09f)); }
	switch (Archetype)
	{
	case ETDZombieArchetype::Crawler:
		Health = 72.0f * WaveHealthScale;
		MoveSpeed = 115.0f;
		OutpostDamage = 2.0f;
		AttackInterval = 2.6f;
		GetMesh()->SetRelativeScale3D(FVector(0.72f, 0.82f, 0.58f));
		if (SalvagePack) { SalvagePack->SetRelativeScale3D(FVector(0.18f, 0.25f, 0.22f)); }
		TintComponent(ChestHarness, FLinearColor(0.24f, 0.29f, 0.08f));
		TintComponent(WarningTag, FLinearColor(0.95f, 0.72f, 0.05f));
		break;
	case ETDZombieArchetype::RadioactiveRunner:
		Health = 62.0f * WaveHealthScale;
		MoveSpeed = 485.0f;
		OutpostDamage = 2.0f;
		AttackInterval = 0.55f;
		GetMesh()->SetRelativeScale3D(FVector(0.9f));
		if (SalvagePack) { SalvagePack->SetRelativeScale3D(FVector(0.2f, 0.25f, 0.27f)); }
		TintComponent(ChestHarness, FLinearColor(0.05f, 0.58f, 0.20f));
		TintComponent(HazardStripe, FLinearColor(0.25f, 1.0f, 0.12f));
		TintComponent(HeadLamp, FLinearColor(0.35f, 1.0f, 0.12f));
		break;
	case ETDZombieArchetype::Brute:
		Health = 320.0f * WaveHealthScale;
		MoveSpeed = 155.0f;
		OutpostDamage = 24.0f;
		AttackInterval = 3.4f;
		GetMesh()->SetRelativeScale3D(FVector(1.3f));
		if (ChestHarness) { ChestHarness->SetRelativeScale3D(FVector(0.42f, 0.36f, 0.36f)); }
		if (ShoulderArmor) { ShoulderArmor->SetRelativeScale3D(FVector(0.32f, 0.49f, 0.24f)); }
		if (SalvagePack) { SalvagePack->SetRelativeScale3D(FVector(0.30f, 0.37f, 0.38f)); }
		if (UtilityBelt) { UtilityBelt->SetRelativeScale3D(FVector(0.44f, 0.34f, 0.12f)); }
		TintComponent(ChestHarness, FLinearColor(0.24f, 0.08f, 0.045f));
		TintComponent(ShoulderArmor, FLinearColor(0.58f, 0.19f, 0.04f));
		break;
	case ETDZombieArchetype::Exploder:
		Health = 68.0f * WaveHealthScale;
		MoveSpeed = 440.0f;
		OutpostDamage = 0.0f;
		GetMesh()->SetRelativeScale3D(FVector(0.94f));
		if (SalvagePack) { SalvagePack->SetRelativeScale3D(FVector(0.30f, 0.32f, 0.36f)); }
		TintComponent(ChestHarness, FLinearColor(0.88f, 0.12f, 0.035f));
		TintComponent(WarningTag, FLinearColor(1.0f, 0.13f, 0.025f));
		TintComponent(HeadLamp, FLinearColor(1.0f, 0.24f, 0.02f));
		break;
	default:
		Health = 70.0f * WaveHealthScale;
		MoveSpeed = 285.0f;
		OutpostDamage = 5.0f;
		break;
	}
	GetCharacterMovement()->MaxWalkSpeed = MoveSpeed;
	GetWorldTimerManager().ClearTimer(AttackTimer);
	if (HasActorBegunPlay() && Archetype != ETDZombieArchetype::Exploder)
	{
		GetWorldTimerManager().SetTimer(AttackTimer, this, &ATDZombieCharacter::AttackOutpost, AttackInterval, true, AttackInterval);
	}
}

float ATDZombieCharacter::TakeDamage(float Damage, const FDamageEvent& DamageEvent,
	AController* EventInstigator, AActor* DamageCauser)
{
	if (bIsDead || Damage <= 0.0f)
	{
		return 0.0f;
	}

	Health = FMath::Max(0.0f, Health - Damage);
	if (Health <= 0.0f)
	{
		Die();
	}
	return Damage;
}

void ATDZombieCharacter::AttackOutpost()
{
	if (!bIsDead && !OutpostTarget.IsNearlyZero()
		&& FVector::DistSquared2D(GetActorLocation(), OutpostTarget) <= FMath::Square(AttackRange))
	{
		if (AextractionFPSGameMode* GameMode = GetWorld()->GetAuthGameMode<AextractionFPSGameMode>())
		{
			GameMode->DamageOutpost(OutpostDamage);
		}
	}
}

void ATDZombieCharacter::ExploderFuseExpired()
{
	if (bIsDead || Archetype != ETDZombieArchetype::Exploder)
	{
		return;
	}
	if (AextractionFPSGameMode* GameMode = GetWorld()->GetAuthGameMode<AextractionFPSGameMode>())
	{
		GameMode->DamageOutpost(ExplosionDamage);
	}
	Die(false);
}

void ATDZombieCharacter::Die(bool bAwardCredits)
{
	if (bIsDead)
	{
		return;
	}

	bIsDead = true;
	GetWorldTimerManager().ClearTimer(AttackTimer);
	GetWorldTimerManager().ClearTimer(ExploderFuseTimer);
	GetCharacterMovement()->StopMovementImmediately();
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	if (AextractionFPSGameMode* GameMode = GetWorld()->GetAuthGameMode<AextractionFPSGameMode>())
	{
		GameMode->NotifyZombieKilled(this, bAwardCredits);
	}
	Destroy();
}
