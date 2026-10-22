// Copyright Epic Games, Inc. All Rights Reserved.

#include "Gameplay/TDTurretPlacementMarker.h"

#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"

ATDTurretPlacementMarker::ATDTurretPlacementMarker()
{
	PrimaryActorTick.bCanEverTick = false;
	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(SceneRoot);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> Cylinder(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> Cone(TEXT("/Engine/BasicShapes/Cone.Cone"));
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> ShapeMaterial(TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));

	ArrowStem = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Arrow Stem"));
	ArrowStem->SetupAttachment(SceneRoot);
	ArrowStem->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	if (Cylinder.Succeeded())
	{
		ArrowStem->SetStaticMesh(Cylinder.Object);
	}
	ArrowStem->SetRelativeLocation(FVector(0.0f, 0.0f, 340.0f));
	ArrowStem->SetRelativeScale3D(FVector(0.16f, 0.16f, 0.9f));

	ArrowHead = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Down Arrow Head"));
	ArrowHead->SetupAttachment(SceneRoot);
	ArrowHead->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	if (Cone.Succeeded())
	{
		ArrowHead->SetStaticMesh(Cone.Object);
	}
	ArrowHead->SetRelativeLocation(FVector(0.0f, 0.0f, 180.0f));
	ArrowHead->SetRelativeRotation(FRotator(180.0f, 0.0f, 0.0f));
	ArrowHead->SetRelativeScale3D(FVector(0.65f, 0.65f, 0.70f));

	if (ShapeMaterial.Succeeded())
	{
		const FLinearColor MarkerColor(1.0f, 0.70f, 0.08f, 1.0f);
		for (UStaticMeshComponent* Part : { ArrowStem.Get(), ArrowHead.Get() })
		{
			if (UMaterialInstanceDynamic* ColorMaterial = UMaterialInstanceDynamic::Create(ShapeMaterial.Object, this))
			{
				ColorMaterial->SetVectorParameterValue(TEXT("Color"), MarkerColor);
				ColorMaterial->SetVectorParameterValue(TEXT("BaseColor"), MarkerColor);
				Part->SetMaterial(0, ColorMaterial);
			}
		}
	}
}
