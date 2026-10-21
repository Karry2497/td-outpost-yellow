// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TDExtractionZone.generated.h"

class UBoxComponent;
class UPrimitiveComponent;

/** Runtime extraction trigger at the helicopter landing zone. */
UCLASS()
class EXTRACTIONFPS_API ATDExtractionZone : public AActor
{
	GENERATED_BODY()

public:
	ATDExtractionZone();
	void SetActive(bool bNewActive);

private:
	UPROPERTY(VisibleAnywhere, Category="Extraction")
	TObjectPtr<UBoxComponent> Trigger;

	UFUNCTION()
	void OnPlayerEntered(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
};
