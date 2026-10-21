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
#include "TimerManager.h"
#include "UObject/ConstructorHelpers.h"

ATDDefenseDeployable::ATDDefenseDeployable()
{
	PrimaryActorTick.bCanEverTick = false;
	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(SceneRoot);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> Cube(TEXT("/Engine/BasicShapes/Cube.Cube"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> Sphere(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> Cylinder(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> Cone(TEXT("/Engine/BasicShapes/Cone.Cone"));
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

	const auto AddPart = [this](
		TArray<TObjectPtr<UStaticMeshComponent>>& Parts, const TCHAR* Name, const TCHAR* Shape,
		const FVector& Location, const FVector& Scale, const FRotator& Rotation, const FLinearColor& Color)
	{
		UStaticMeshComponent* Part = CreateDefaultSubobject<UStaticMeshComponent>(FName(Name));
		Part->SetupAttachment(SceneRoot);
		Part->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		if (FCString::Stricmp(Shape, TEXT("Sphere")) == 0 && Sphere.Succeeded())
		{
			Part->SetStaticMesh(Sphere.Object);
		}
		else if (FCString::Stricmp(Shape, TEXT("Cylinder")) == 0 && Cylinder.Succeeded())
		{
			Part->SetStaticMesh(Cylinder.Object);
		}
		else if (FCString::Stricmp(Shape, TEXT("Cone")) == 0 && Cone.Succeeded())
		{
			Part->SetStaticMesh(Cone.Object);
		}
		else if (Cube.Succeeded())
		{
			Part->SetStaticMesh(Cube.Object);
		}
		Part->SetRelativeLocation(Location);
		Part->SetRelativeScale3D(Scale);
		Part->SetRelativeRotation(Rotation);
		if (ShapeMaterial.Succeeded())
		{
			if (UMaterialInstanceDynamic* ColorMaterial = UMaterialInstanceDynamic::Create(ShapeMaterial.Object, this))
			{
				ColorMaterial->SetVectorParameterValue(TEXT("Color"), Color);
				ColorMaterial->SetVectorParameterValue(TEXT("BaseColor"), Color);
				Part->SetMaterial(0, ColorMaterial);
			}
		}
		Parts.Add(Part);
		return Part;
	};

	// Turret: broad planted base, armored rotating housing, twin barrels and a
	// bright sight. Low, separated layers remain readable from the locked camera.
	AddPart(TurretModelParts, TEXT("Turret Footplate"), TEXT("Cube"), FVector(0, 0, 18), FVector(1.65f, 1.65f, 0.24f), FRotator(0, 0, 0), FLinearColor(0.045f, 0.09f, 0.12f));
	AddPart(TurretModelParts, TEXT("Turret Bearing"), TEXT("Cylinder"), FVector(0, 0, 43), FVector(1.12f, 1.12f, 0.25f), FRotator(0, 0, 0), FLinearColor(0.09f, 0.20f, 0.23f));
	AddPart(TurretModelParts, TEXT("Turret Armored Hull"), TEXT("Cube"), FVector(0, 0, 83), FVector(1.10f, 0.88f, 0.62f), FRotator(0, 0, 0), FLinearColor(0.05f, 0.42f, 0.48f));
	AddPart(TurretModelParts, TEXT("Turret Left Armor"), TEXT("Cube"), FVector(-57, 0, 88), FVector(0.22f, 0.99f, 0.53f), FRotator(0, 0, -7), FLinearColor(0.04f, 0.26f, 0.31f));
	AddPart(TurretModelParts, TEXT("Turret Right Armor"), TEXT("Cube"), FVector(57, 0, 88), FVector(0.22f, 0.99f, 0.53f), FRotator(0, 0, 7), FLinearColor(0.04f, 0.26f, 0.31f));
	AddPart(TurretModelParts, TEXT("Turret Top Panel"), TEXT("Cube"), FVector(-5, 0, 145), FVector(0.84f, 0.69f, 0.16f), FRotator(0, 0, 0), FLinearColor(1.0f, 0.32f, 0.045f));
	AddPart(TurretModelParts, TEXT("Turret Barrel Left"), TEXT("Cylinder"), FVector(122, -27, 91), FVector(0.15f, 0.15f, 0.80f), FRotator(90, 0, 0), FLinearColor(0.10f, 0.14f, 0.16f));
	AddPart(TurretModelParts, TEXT("Turret Barrel Right"), TEXT("Cylinder"), FVector(122, 27, 91), FVector(0.15f, 0.15f, 0.80f), FRotator(90, 0, 0), FLinearColor(0.10f, 0.14f, 0.16f));
	AddPart(TurretModelParts, TEXT("Turret Muzzle Left"), TEXT("Cylinder"), FVector(196, -27, 91), FVector(0.19f, 0.19f, 0.14f), FRotator(90, 0, 0), FLinearColor(1.0f, 0.55f, 0.08f));
	AddPart(TurretModelParts, TEXT("Turret Muzzle Right"), TEXT("Cylinder"), FVector(196, 27, 91), FVector(0.19f, 0.19f, 0.14f), FRotator(90, 0, 0), FLinearColor(1.0f, 0.55f, 0.08f));
	AddPart(TurretModelParts, TEXT("Turret Optic"), TEXT("Sphere"), FVector(36, 0, 154), FVector(0.19f, 0.19f, 0.19f), FRotator(0, 0, 0), FLinearColor(0.12f, 1.0f, 0.78f));
	AddPart(TurretModelParts, TEXT("Turret Antenna"), TEXT("Cylinder"), FVector(-58, 52, 161), FVector(0.035f, 0.035f, 0.32f), FRotator(0, 0, 0), FLinearColor(0.84f, 0.90f, 0.82f));

	// Compact tracked guard drone with a distinct head, sensor, side wheels and guns.
	AddPart(BotModelParts, TEXT("Bot Lower Chassis"), TEXT("Cube"), FVector(0, 0, 24), FVector(1.25f, 0.74f, 0.24f), FRotator(0, 0, 0), FLinearColor(0.08f, 0.16f, 0.18f));
	AddPart(BotModelParts, TEXT("Bot Left Track"), TEXT("Cube"), FVector(0, -75, 34), FVector(1.27f, 0.23f, 0.36f), FRotator(0, 0, 0), FLinearColor(0.07f, 0.08f, 0.085f));
	AddPart(BotModelParts, TEXT("Bot Right Track"), TEXT("Cube"), FVector(0, 75, 34), FVector(1.27f, 0.23f, 0.36f), FRotator(0, 0, 0), FLinearColor(0.07f, 0.08f, 0.085f));
	for (int32 WheelIndex = 0; WheelIndex < 3; ++WheelIndex)
	{
		const float WheelX = -72.0f + WheelIndex * 72.0f;
		AddPart(BotModelParts, *FString::Printf(TEXT("Bot Left Road Wheel %d"), WheelIndex), TEXT("Cylinder"), FVector(WheelX, -89, 35), FVector(0.21f, 0.21f, 0.10f), FRotator(0, 0, 90), FLinearColor(0.94f, 0.36f, 0.05f));
		AddPart(BotModelParts, *FString::Printf(TEXT("Bot Right Road Wheel %d"), WheelIndex), TEXT("Cylinder"), FVector(WheelX, 89, 35), FVector(0.21f, 0.21f, 0.10f), FRotator(0, 0, 90), FLinearColor(0.94f, 0.36f, 0.05f));
	}
	AddPart(BotModelParts, TEXT("Bot Armored Body"), TEXT("Cube"), FVector(-4, 0, 76), FVector(0.88f, 0.67f, 0.43f), FRotator(0, 0, 0), FLinearColor(0.04f, 0.49f, 0.48f));
	AddPart(BotModelParts, TEXT("Bot Chest Plate"), TEXT("Cube"), FVector(48, 0, 75), FVector(0.18f, 0.51f, 0.29f), FRotator(0, 0, -5), FLinearColor(0.96f, 0.32f, 0.035f));
	AddPart(BotModelParts, TEXT("Bot Head"), TEXT("Cube"), FVector(0, 0, 127), FVector(0.49f, 0.45f, 0.34f), FRotator(0, 0, 0), FLinearColor(0.11f, 0.23f, 0.25f));
	AddPart(BotModelParts, TEXT("Bot Visor"), TEXT("Cube"), FVector(45, 0, 132), FVector(0.10f, 0.32f, 0.11f), FRotator(0, 0, 0), FLinearColor(0.14f, 1.0f, 0.76f));
	AddPart(BotModelParts, TEXT("Bot Gun Left"), TEXT("Cylinder"), FVector(108, -29, 75), FVector(0.12f, 0.12f, 0.55f), FRotator(90, 0, 0), FLinearColor(0.09f, 0.12f, 0.13f));
	AddPart(BotModelParts, TEXT("Bot Gun Right"), TEXT("Cylinder"), FVector(108, 29, 75), FVector(0.12f, 0.12f, 0.55f), FRotator(90, 0, 0), FLinearColor(0.09f, 0.12f, 0.13f));
	AddPart(BotModelParts, TEXT("Bot Rear Battery"), TEXT("Cube"), FVector(-70, 0, 111), FVector(0.29f, 0.44f, 0.20f), FRotator(0, 0, 0), FLinearColor(0.99f, 0.67f, 0.09f));

	// Readable anti-infantry obstacle: marked concrete anchors, two steel rails,
	// repeated angled wire struts and outward-facing barbs along each span.
	AddPart(WireModelParts, TEXT("Wire Long Base"), TEXT("Cube"), FVector(0, 0, 18), FVector(1.65f, 0.12f, 0.16f), FRotator(0, 0, 0), FLinearColor(0.36f, 0.28f, 0.19f));
	AddPart(WireModelParts, TEXT("Wire Rail Lower"), TEXT("Cylinder"), FVector(0, 0, 30), FVector(0.055f, 0.055f, 1.52f), FRotator(90, 0, 0), FLinearColor(0.42f, 0.48f, 0.44f));
	AddPart(WireModelParts, TEXT("Wire Rail Upper"), TEXT("Cylinder"), FVector(0, 0, 82), FVector(0.055f, 0.055f, 1.52f), FRotator(90, 0, 0), FLinearColor(0.42f, 0.48f, 0.44f));
	for (int32 BarbIndex = 0; BarbIndex < 7; ++BarbIndex)
	{
		const float X = -135.0f + BarbIndex * 45.0f;
		const float Lean = BarbIndex % 2 == 0 ? -18.0f : 18.0f;
		AddPart(WireModelParts, *FString::Printf(TEXT("Wire Diagonal %d"), BarbIndex), TEXT("Cylinder"), FVector(X, 0, 56), FVector(0.035f, 0.035f, 0.34f), FRotator(0, Lean, 90), FLinearColor(0.53f, 0.58f, 0.52f));
		AddPart(WireModelParts, *FString::Printf(TEXT("Wire Barb %d"), BarbIndex), TEXT("Cone"), FVector(X, 0, 56), FVector(0.10f, 0.10f, 0.20f), FRotator(0, 0, BarbIndex % 2 == 0 ? 90.0f : -90.0f), FLinearColor(0.98f, 0.57f, 0.12f));
	}
	for (int32 EndIndex = 0; EndIndex < 2; ++EndIndex)
	{
		const float X = EndIndex == 0 ? -150.0f : 150.0f;
		AddPart(WireModelParts, *FString::Printf(TEXT("Wire End Stake %d"), EndIndex), TEXT("Cylinder"), FVector(X, 0, 49), FVector(0.13f, 0.13f, 0.53f), FRotator(0, 0, 0), FLinearColor(0.94f, 0.32f, 0.035f));
	}
}

void ATDDefenseDeployable::Configure(ETDDefenseType NewType)
{
	DefenseType = NewType;
	BaseMesh->SetVisibility(false, true);
	TopMesh->SetVisibility(false, true);
	for (UStaticMeshComponent* Part : TurretModelParts) { Part->SetVisibility(DefenseType == ETDDefenseType::Turret, true); }
	for (UStaticMeshComponent* Part : BotModelParts) { Part->SetVisibility(DefenseType == ETDDefenseType::GuardBot, true); }
	for (UStaticMeshComponent* Part : WireModelParts) { Part->SetVisibility(DefenseType == ETDDefenseType::BarbedWire, true); }
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
	ScheduleAttackTimer();
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
	ScheduleAttackTimer();
	if (TurretModelParts.IsValidIndex(5))
	{
		UStaticMeshComponent* UpgradePanel = TurretModelParts[5];
		UpgradePanel->SetRelativeScale3D(FVector(0.84f + 0.07f * UpgradeLevel, 0.69f + 0.06f * UpgradeLevel, 0.16f));
		UpgradePanel->SetRelativeLocation(FVector(-5.0f, 0.0f, 145.0f + 8.0f * UpgradeLevel));
		if (UMaterialInstanceDynamic* DynamicMaterial = Cast<UMaterialInstanceDynamic>(UpgradePanel->GetMaterial(0)))
		{
			const FLinearColor TierColor = UpgradeLevel == 1
				? FLinearColor(1.0f, 0.72f, 0.08f)
				: (UpgradeLevel == 2 ? FLinearColor(1.0f, 0.36f, 0.04f) : FLinearColor(0.2f, 1.0f, 0.35f));
			DynamicMaterial->SetVectorParameterValue(TEXT("Color"), TierColor);
			DynamicMaterial->SetVectorParameterValue(TEXT("BaseColor"), TierColor);
		}
	}

	return true;
}

void ATDDefenseDeployable::ScheduleAttackTimer()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(AttackTimer);
		World->GetTimerManager().SetTimer(AttackTimer, this,
			&ATDDefenseDeployable::AttackNearestZombie, AttackInterval, true, AttackInterval);
	}
	else
	{
		AttackTimer.Invalidate();
	}
}

void ATDDefenseDeployable::AttackNearestZombie()
{
	if (!GetWorld())
	{
		return;
	}

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
