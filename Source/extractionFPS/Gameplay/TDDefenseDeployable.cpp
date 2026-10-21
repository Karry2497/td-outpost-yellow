// Copyright Epic Games, Inc. All Rights Reserved.

#include "Gameplay/TDDefenseDeployable.h"

#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/DamageEvents.h"
#include "Engine/World.h"
#include "Gameplay/TDZombieCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"

ATDDefenseDeployable::ATDDefenseDeployable()
{
	PrimaryActorTick.bCanEverTick = true;
	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(SceneRoot);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> Cube(TEXT("/Engine/BasicShapes/Cube.Cube"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> Sphere(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> ShapeMaterial(TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
	BaseMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Defense Body"));
	BaseMesh->SetupAttachment(SceneRoot);
	BaseMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	TopMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Defense Head"));
	TopMesh->SetupAttachment(BaseMesh);
	TopMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	if (Cube.Succeeded())
	{
		BaseMesh->SetStaticMesh(Cube.Object);
		TopMesh->SetStaticMesh(Sphere.Succeeded() ? Sphere.Object : Cube.Object);
	}
	if (ShapeMaterial.Succeeded())
	{
		UMaterialInstanceDynamic* BaseColor = UMaterialInstanceDynamic::Create(ShapeMaterial.Object, this);
		if (BaseColor)
		{
			BaseColor->SetVectorParameterValue(TEXT("Color"), FLinearColor(0.02f, 0.8f, 0.9f));
			BaseColor->SetVectorParameterValue(TEXT("BaseColor"), FLinearColor(0.02f, 0.8f, 0.9f));
			BaseMesh->SetMaterial(0, BaseColor);
		}
		UMaterialInstanceDynamic* TopColor = UMaterialInstanceDynamic::Create(ShapeMaterial.Object, this);
		if (TopColor)
		{
			TopColor->SetVectorParameterValue(TEXT("Color"), FLinearColor(1.0f, 0.24f, 0.035f));
			TopColor->SetVectorParameterValue(TEXT("BaseColor"), FLinearColor(1.0f, 0.24f, 0.035f));
			TopMesh->SetMaterial(0, TopColor);
		}
	}
}

void ATDDefenseDeployable::Configure(ETDDefenseType NewType)
{
	DefenseType = NewType;
	switch (DefenseType)
	{
	case ETDDefenseType::GuardBot:
		AttackInterval = 0.5f;
		AttackRadius = 1350.0f;
		AttackDamage = 18.0f;
		BaseMesh->SetRelativeLocation(FVector(0, 0, 65));
		BaseMesh->SetRelativeScale3D(FVector(0.38f, 0.38f, 0.65f));
		TopMesh->SetRelativeLocation(FVector(0, 0, 0.9f));
		TopMesh->SetRelativeScale3D(FVector(0.8f));
		break;
	case ETDDefenseType::BarbedWire:
		AttackInterval = 0.7f;
		AttackRadius = 300.0f;
		AttackDamage = 8.0f;
		BaseMesh->SetRelativeLocation(FVector(0, 0, 12));
		BaseMesh->SetRelativeScale3D(FVector(2.2f, 0.45f, 0.12f));
		TopMesh->SetRelativeLocation(FVector(0, 0, 0.75f));
		TopMesh->SetRelativeScale3D(FVector(0.35f, 2.0f, 0.2f));
		break;
	default:
		AttackInterval = 0.75f;
		AttackRadius = 2000.0f;
		AttackDamage = 30.0f;
		BaseMesh->SetRelativeLocation(FVector(0, 0, 48));
		BaseMesh->SetRelativeScale3D(FVector(0.55f, 0.55f, 0.48f));
		TopMesh->SetRelativeLocation(FVector(0, 0, 1.05f));
		TopMesh->SetRelativeScale3D(FVector(0.6f, 0.6f, 0.35f));
		break;
	}
}

bool ATDDefenseDeployable::Upgrade()
{
	if (!CanUpgrade())
	{
		return false;
	}

	++UpgradeLevel;
	AttackDamage *= 1.35f;
	AttackInterval = FMath::Max(0.28f, AttackInterval * 0.88f);
	AttackRadius *= 1.10f;
	TopMesh->SetRelativeScale3D(FVector(0.6f + 0.09f * UpgradeLevel));
	TopMesh->SetRelativeLocation(FVector(0.0f, 0.0f, 1.05f + 0.12f * UpgradeLevel));

	if (UMaterialInstanceDynamic* DynamicMaterial = Cast<UMaterialInstanceDynamic>(TopMesh->GetMaterial(0)))
	{
		const FLinearColor TierColor = UpgradeLevel == 1
			? FLinearColor(1.0f, 0.72f, 0.08f)
			: (UpgradeLevel == 2 ? FLinearColor(1.0f, 0.36f, 0.04f) : FLinearColor(0.2f, 1.0f, 0.35f));
		DynamicMaterial->SetVectorParameterValue(TEXT("Color"), TierColor);
		DynamicMaterial->SetVectorParameterValue(TEXT("BaseColor"), TierColor);
	}
	return true;
}

void ATDDefenseDeployable::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	TickAccumulator += DeltaSeconds;
	if (TickAccumulator < AttackInterval || !GetWorld())
	{
		return;
	}
	TickAccumulator = 0.0f;

	TArray<AActor*> Zombies;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ATDZombieCharacter::StaticClass(), Zombies);
	ATDZombieCharacter* Target = nullptr;
	float BestDistance = FMath::Square(AttackRadius);
	for (AActor* Actor : Zombies)
	{
		ATDZombieCharacter* Zombie = Cast<ATDZombieCharacter>(Actor);
		if (!Zombie)
		{
			continue;
		}
		const float Distance = FVector::DistSquared2D(GetActorLocation(), Zombie->GetActorLocation());
		if (Distance < BestDistance)
		{
			BestDistance = Distance;
			Target = Zombie;
		}
	}
	if (Target)
	{
		const FVector Direction = Target->GetActorLocation() - GetActorLocation();
		SetActorRotation(FRotator(0.0f, Direction.Rotation().Yaw, 0.0f));
		Target->TakeDamage(AttackDamage, FDamageEvent(), nullptr, this);
	}
}
