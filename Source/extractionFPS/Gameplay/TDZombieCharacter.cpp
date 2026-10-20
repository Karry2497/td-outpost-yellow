// Copyright Epic Games, Inc. All Rights Reserved.

#include "Gameplay/TDZombieCharacter.h"

#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "GameFramework/CharacterMovementComponent.h"
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

	static ConstructorHelpers::FObjectFinder<USkeletalMesh> ZombieMesh(
		TEXT("/Game/Characters/Mannequins/Meshes/SKM_Manny_Simple.SKM_Manny_Simple"));
	if (ZombieMesh.Succeeded())
	{
		GetMesh()->SetSkeletalMesh(ZombieMesh.Object);
		GetMesh()->SetRelativeLocationAndRotation(FVector(0.0f, 0.0f, -92.0f), FRotator(0.0f, -90.0f, 0.0f));
	}

	static ConstructorHelpers::FClassFinder<UAnimInstance> ZombieAnim(
		TEXT("/Game/Characters/Mannequins/Anims/Unarmed/ABP_Unarmed"));
	if (ZombieAnim.Succeeded())
	{
		GetMesh()->SetAnimInstanceClass(ZombieAnim.Class);
	}

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> ShapeMaterial(
		TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));

	if (CubeMesh.Succeeded() && ShapeMaterial.Succeeded())
	{
		ChestHarness = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Graphic Chest Harness"));
		ChestHarness->SetupAttachment(GetMesh(), TEXT("spine_03"));
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
			ShoulderArmor->SetupAttachment(GetMesh(), TEXT("spine_03"));
			ShoulderArmor->SetStaticMesh(SphereMesh.Object);
			ShoulderArmor->SetRelativeLocation(FVector(0.0f, 0.0f, 22.0f));
			ShoulderArmor->SetRelativeScale3D(FVector(0.23f, 0.39f, 0.16f));
			ShoulderArmor->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			ShoulderArmor->SetMaterial(0, MakeFlatColor(ShapeMaterial.Object, this, FLinearColor(1.0f, 0.12f, 0.025f)));
		}

		for (int32 MaterialIndex = 0; MaterialIndex < GetMesh()->GetNumMaterials(); ++MaterialIndex)
		{
			GetMesh()->SetMaterial(MaterialIndex,
				MakeFlatColor(ShapeMaterial.Object, this, FLinearColor(1.0f, 0.84f, 0.015f)));
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
	GetWorldTimerManager().SetTimer(AttackTimer, this, &ATDZombieCharacter::AttackOutpost, 1.2f, true, 1.2f);
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
		AddMovementInput(ToTarget.GetSafeNormal2D(), 1.0f);
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
		OutpostDamage = 12.0f;
		GetMesh()->SetRelativeScale3D(FVector(0.88f));
		break;
	case ETDZombieArchetype::Brute:
		Health = 220.0f * WaveHealthScale;
		MoveSpeed = 185.0f;
		OutpostDamage = 42.0f;
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
		OutpostDamage = 18.0f;
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
