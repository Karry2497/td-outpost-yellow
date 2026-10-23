// Copyright Epic Games, Inc. All Rights Reserved.

#include "Gameplay/TDDefenseProjectile.h"

#include "Components/StaticMeshComponent.h"
#include "Engine/DamageEvents.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "Gameplay/TDZombieCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "TimerManager.h"
#include "UObject/ConstructorHelpers.h"

ATDDefenseProjectile::ATDDefenseProjectile()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickInterval = 0.05f;
	static ConstructorHelpers::FObjectFinder<UStaticMesh> Sphere(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> Cylinder(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> ShapeMaterial(TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
	ProjectileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Projectile Mesh"));
	SetRootComponent(ProjectileMesh);
	ProjectileMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ProjectileMesh->SetStaticMesh(Sphere.Object);
	if (ShapeMaterial.Succeeded())
	{
		if (UMaterialInstanceDynamic* Material = UMaterialInstanceDynamic::Create(ShapeMaterial.Object, this))
		{
			Material->SetVectorParameterValue(TEXT("Color"), FLinearColor(1.0f, 0.52f, 0.04f));
			Material->SetVectorParameterValue(TEXT("BaseColor"), FLinearColor(1.0f, 0.52f, 0.04f));
			ProjectileMesh->SetMaterial(0, Material);
		}
	}
	if (Cylinder.Succeeded()) ProjectileMesh->SetStaticMesh(Cylinder.Object);
}

void ATDDefenseProjectile::Configure(const FVector& Start, const FVector& Landing, float FlightSeconds, float ArcHeight,
	float Damage, float SplashRadius, bool bAsMine)
{
	StartLocation = Start;
	LandingLocation = Landing;
	FlightDuration = FMath::Max(0.25f, FlightSeconds);
	ArcHeightCm = ArcHeight;
	DamageAmount = Damage;
	SplashRadiusCm = SplashRadius;
	bMine = bAsMine;
	SetActorLocation(StartLocation);
	ProjectileMesh->SetWorldScale3D(bMine ? FVector(0.30f, 0.30f, 0.44f) : FVector(0.30f));
}

void ATDDefenseProjectile::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (!bLanded)
	{
		ElapsedFlight += DeltaSeconds;
		const float Alpha = FMath::Clamp(ElapsedFlight / FlightDuration, 0.0f, 1.0f);
		FVector Position = FMath::Lerp(StartLocation, LandingLocation, Alpha);
		Position.Z += FMath::Sin(Alpha * PI) * ArcHeightCm;
		SetActorLocation(Position);
		if (Alpha >= 1.0f)
		{
			bLanded = true;
			SetActorLocation(LandingLocation);
			if (!bMine) { Detonate(); return; }
			PrimaryActorTick.TickInterval = 0.2f;
			ProjectileMesh->SetWorldScale3D(FVector(0.46f, 0.46f, 0.16f));
		}
		return;
	}

	TimeSinceLanding += DeltaSeconds;
	if (!bArmed && TimeSinceLanding >= 1.0f) bArmed = true;
	if (bArmed)
	{
		TArray<AActor*> Zombies;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), ATDZombieCharacter::StaticClass(), Zombies);
		for (AActor* Actor : Zombies)
		{
			if (ATDZombieCharacter* Zombie = Cast<ATDZombieCharacter>(Actor))
			{
				if (FVector::DistSquared2D(Zombie->GetActorLocation(), LandingLocation) <= FMath::Square(190.0f))
				{
					Detonate();
					return;
				}
			}
		}
	}
	if (TimeSinceLanding >= MineLifetime) Destroy();
}

void ATDDefenseProjectile::Detonate()
{
	if (!GetWorld()) { Destroy(); return; }
	TArray<AActor*> Zombies;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ATDZombieCharacter::StaticClass(), Zombies);
	const float RadiusSquared = FMath::Square(SplashRadiusCm);
	for (AActor* Actor : Zombies)
	{
		ATDZombieCharacter* Zombie = Cast<ATDZombieCharacter>(Actor);
		if (Zombie && FVector::DistSquared2D(Zombie->GetActorLocation(), LandingLocation) <= RadiusSquared)
		{
			Zombie->TakeDamage(DamageAmount, FDamageEvent(), nullptr, this);
		}
	}
	Destroy();
}
