// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TDExtractionGate.generated.h"

class UStaticMeshComponent;

/** A physical barrier that stays shut through the defense and opens for evac. */
UCLASS()
class EXTRACTIONFPS_API ATDExtractionGate : public AActor
{
	GENERATED_BODY()

public:
	ATDExtractionGate();
	void OpenGate();

private:
	UPROPERTY(VisibleAnywhere, Category="Extraction")
	TObjectPtr<UStaticMeshComponent> GateMesh;
};
