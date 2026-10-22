// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TDTurretPlacementMarker.generated.h"

class UStaticMeshComponent;
class USceneComponent;

/** Bright, downward-pointing in-world marker for an available rooftop turret pad. */
UCLASS()
class EXTRACTIONFPS_API ATDTurretPlacementMarker : public AActor
{
	GENERATED_BODY()

public:
	ATDTurretPlacementMarker();

private:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> ArrowStem;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> ArrowHead;
};
