// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "TimerManager.h"
#include "extractionFPSGameMode.generated.h"

class ATDZombieCharacter;
class ATDDefenseDeployable;
class ATDExtractionGate;
class ATDExtractionZone;
class APawn;

UENUM(BlueprintType)
enum class ETDDefenseType : uint8
{
	Turret,
	GuardBot,
	BarbedWire
};

/**
 *  Simple GameMode for a first person game
 */
UCLASS(abstract)
class AextractionFPSGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AextractionFPSGameMode();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	void StartNextWave();
	void SpawnNextZombie();
	void CheckForWaveCompletion();
	void RefreshDefenseHUD();
	void UnlockExtraction();

	FTimerHandle WaveTimer;
	FTimerHandle SpawnTimer;
	FTimerHandle HUDTimer;
	TSet<TObjectPtr<ATDZombieCharacter>> ActiveZombies;
	int32 CurrentWave = 0;
	int32 ZombiesLeftToSpawn = 0;
	int32 Credits = 200;
	float OutpostHealth = 1000.0f;
	float OutpostDamageMultiplier = 1.0f;
	int32 WallUpgradeLevel = 0;
	int32 NextSpawnLane = 0;
	static constexpr int32 MaxWaves = 5;
	bool bOutpostOverrun = false;
	bool bExtractionUnlocked = false;
	bool bLevelComplete = false;
	TObjectPtr<ATDExtractionZone> ExtractionZone;
	TArray<TObjectPtr<ATDExtractionGate>> ExtractionGates;

public:
	void NotifyZombieKilled(ATDZombieCharacter* Zombie);
	void DamageOutpost(float Damage);
	bool PurchaseDefense(ETDDefenseType Type, const FVector& Location, const FRotator& Rotation);
	bool UpgradeNearestTower(const FVector& Location);
	bool RepairOutpost();
	bool UpgradeOutpostWalls();
	bool CallAirstrike(const FVector& Center);
	bool RefillPlayerAmmo(APawn* PlayerPawn);
	void CompleteExtraction();
	void ShowDefenseStatus(const FString& Message, const FColor& Color = FColor::White) const;

	UFUNCTION(BlueprintPure, Category="Defense")
	int32 GetCurrentWave() const { return CurrentWave; }

	UFUNCTION(BlueprintPure, Category="Defense")
	int32 GetCredits() const { return Credits; }

	UFUNCTION(BlueprintPure, Category="Defense")
	float GetOutpostHealth() const { return OutpostHealth; }
};



