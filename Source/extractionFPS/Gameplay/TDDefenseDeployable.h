// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TimerManager.h"
#include "extractionFPSGameMode.h"
#include "TDDefenseDeployable.generated.h"

class UStaticMeshComponent;
class USceneComponent;

/** Stylized, readable tower-defense defenses assembled from reusable mesh parts. */
UCLASS()
class EXTRACTIONFPS_API ATDDefenseDeployable : public AActor
{
	GENERATED_BODY()

public:
	ATDDefenseDeployable();
	void Configure(ETDDefenseType NewType);
	bool Upgrade();
	bool IsTurret() const { return DefenseType == ETDDefenseType::Turret; }
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

	UPROPERTY(VisibleAnywhere, Category="Defense|Model")
	TArray<TObjectPtr<UStaticMeshComponent>> TurretModelParts;

	UPROPERTY(VisibleAnywhere, Category="Defense|Model")
	TArray<TObjectPtr<UStaticMeshComponent>> BotModelParts;

	UPROPERTY(VisibleAnywhere, Category="Defense|Model")
	TArray<TObjectPtr<UStaticMeshComponent>> WireModelParts;

	ETDDefenseType DefenseType = ETDDefenseType::Turret;
	FTimerHandle AttackTimer;
	float AttackInterval = 0.75f;
	float AttackRadius = 2000.0f;
	float AttackDamage = 30.0f;
	int32 UpgradeLevel = 0;
};
