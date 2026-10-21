// Copyright Epic Games, Inc. All Rights Reserved.

#include "Gameplay/TDExtractionGate.h"

#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"

ATDExtractionGate::ATDExtractionGate()
{
	PrimaryActorTick.bCanEverTick = false;
	GateMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Gate Mesh"));
	SetRootComponent(GateMesh);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> Cube(TEXT("/Engine/BasicShapes/Cube.Cube"));
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> ShapeMaterial(
		TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
	if (Cube.Succeeded())
	{
		GateMesh->SetStaticMesh(Cube.Object);
		GateMesh->SetCollisionProfileName(TEXT("BlockAll"));
	}
	if (ShapeMaterial.Succeeded())
	{
		// Use a persistent material on editor-placed gates. Dynamic materials
		// created in the constructor are transient and prevent the level saving.
		GateMesh->SetMaterial(0, ShapeMaterial.Object);
	}
}

void ATDExtractionGate::OpenGate()
{
	SetActorEnableCollision(false);
	SetActorHiddenInGame(true);
}
