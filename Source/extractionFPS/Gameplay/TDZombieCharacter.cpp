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
		TEXT("/Game/Maps/Materials/M_Zombie_StylizedYellow.M_Zombie_StylizedYellow"));

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

	GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GetMesh()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	GetCharacterMovement()->MaxWalkSpeed = MoveSpeed;
	GetCharacterMovement()->bOrientRotationToMovement = true;
	bUseControllerRotationYaw = false;
}

void ATDZombieCharacter::BeginPlay()
{
	Super::BeginPlay();
	GetCharacterMovement()->MaxWalkSpeed = MoveSpeed;
	GetWorldTimerManager().SetTimer(AttackTimer, this, &ATDZombieCharacter::AttackOutpost, 2.0f, true, 2.0f);
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
	switch (Archetype)
	{
	case ETDZombieArchetype::Runner:
		Health = 48.0f * WaveHealthScale;
		MoveSpeed = 430.0f;
		OutpostDamage = 3.0f;
		GetMesh()->SetRelativeScale3D(FVector(0.88f));
		break;
	case ETDZombieArchetype::Brute:
		Health = 220.0f * WaveHealthScale;
		MoveSpeed = 185.0f;
		OutpostDamage = 12.0f;
		GetMesh()->SetRelativeScale3D(FVector(1.22f));
		if (ChestHarness)
		{
			ChestHarness->SetRelativeScale3D(FVector(0.40f, 0.35f, 0.34f));
		}
		if (ShoulderArmor)
		{
			ShoulderArmor->SetRelativeScale3D(FVector(0.31f, 0.48f, 0.23f));
		}
		break;
	default:
		Health = 70.0f * WaveHealthScale;
		MoveSpeed = 285.0f;
		OutpostDamage = 5.0f;
		GetMesh()->SetRelativeScale3D(FVector(1.0f));
		break;
	}
	GetCharacterMovement()->MaxWalkSpeed = MoveSpeed;
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

void ATDZombieCharacter::Die()
{
	if (bIsDead)
	{
		return;
	}

	bIsDead = true;
	GetWorldTimerManager().ClearTimer(AttackTimer);
	GetCharacterMovement()->StopMovementImmediately();
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	if (AextractionFPSGameMode* GameMode = GetWorld()->GetAuthGameMode<AextractionFPSGameMode>())
	{
		GameMode->NotifyZombieKilled(this);
	}
	Destroy();
}
