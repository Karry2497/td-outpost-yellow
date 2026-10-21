// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TimerManager.h"
#include "extractionFPSGameMode.h"
#include "TDDefenseDeployable.generated.h"

class UStaticMeshComponent;
class USceneComponent;

/** Small, readable placeholder defense actors for the first playable loop. */
UCLASS()
class EXTRACTIONFPS_API ATDDefenseDeployable : public AActor
{
	GENERATED_BODY()

public:
	ATDDefenseDeployable();
	void Configure(ETDDefenseType NewType);
	bool Upgrade();
	int32 GetUpgradeLevel() const { return UpgradeLevel; }
	bool CanUpgrade() const { return DefenseType == ETDDefenseType::Turret && UpgradeLevel < 3; }

protected:
	void AttackNearestZombie();
	void ScheduleAttackTimer();

private:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> BaseMesh;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> TopMesh;

	ETDDefenseType DefenseType = ETDDefenseType::Turret;
	FTimerHandle AttackTimer;
	float AttackInterval = 0.75f;
	float AttackRadius = 2000.0f;
	float AttackDamage = 30.0f;
	int32 UpgradeLevel = 0;
};
