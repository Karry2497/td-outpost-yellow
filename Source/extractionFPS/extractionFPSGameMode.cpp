// Copyright Epic Games, Inc. All Rights Reserved.

#include "extractionFPSGameMode.h"
#include "Gameplay/TDZombieCharacter.h"
#include "Gameplay/TDDefenseDeployable.h"
#include "Gameplay/TDExtractionGate.h"
#include "Gameplay/TDExtractionZone.h"
#include "Variant_Shooter/ShooterCharacter.h"
#include "Variant_Shooter/Weapons/ShooterWeapon.h"
#include "Engine/Engine.h"
#include "Engine/DamageEvents.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

AextractionFPSGameMode::AextractionFPSGameMode()
{
	// The existing Blueprint game mode continues to provide the player's pawn
	// and input assets. This native class owns the tower-defense wave loop.
}

void AextractionFPSGameMode::BeginPlay()
{
	Super::BeginPlay();

	TArray<AActor*> GateActors;
	UGameplayStatics::GetAllActorsOfClass(this, ATDExtractionGate::StaticClass(), GateActors);
	for (AActor* Actor : GateActors)
	{
		if (ATDExtractionGate* Gate = Cast<ATDExtractionGate>(Actor))
		{
			ExtractionGates.Add(Gate);
		}
	}
	ExtractionZone = GetWorld()->SpawnActor<ATDExtractionZone>(
		ATDExtractionZone::StaticClass(), FVector(0.0f, -12000.0f, 120.0f), FRotator::ZeroRotator);

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 6.0f, FColor::Yellow,
			TEXT("Protect the outpost. Spend credits on defenses, repairs and supplies."));
	}

	GetWorldTimerManager().SetTimer(WaveTimer, this, &AextractionFPSGameMode::StartNextWave, 8.0f, false);
	GetWorldTimerManager().SetTimer(HUDTimer, this, &AextractionFPSGameMode::RefreshDefenseHUD, 0.5f, true, 0.0f);
}

void AextractionFPSGameMode::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GetWorldTimerManager().ClearTimer(WaveTimer);
	GetWorldTimerManager().ClearTimer(SpawnTimer);
	GetWorldTimerManager().ClearTimer(HUDTimer);
	Super::EndPlay(EndPlayReason);
}

void AextractionFPSGameMode::StartNextWave()
{
	if (bOutpostOverrun || bLevelComplete || bExtractionUnlocked || CurrentWave >= MaxWaves)
	{
		return;
	}
	++CurrentWave;
	ZombiesLeftToSpawn = 4 + CurrentWave * 2;

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 4.0f, FColor(255, 220, 0),
			FString::Printf(TEXT("WAVE %d  |  %d zombies inbound"), CurrentWave, ZombiesLeftToSpawn));
	}

	SpawnNextZombie();
}

void AextractionFPSGameMode::SpawnNextZombie()
{
	if (ZombiesLeftToSpawn <= 0 || !GetWorld())
	{
		CheckForWaveCompletion();
		return;
	}

	// The three approaches stop at the outpost walls: west, east and south.
	static const FVector SpawnLocations[] = {
		FVector(-6880.0f, -280.0f, 100.0f),
		FVector(6880.0f, 280.0f, 100.0f),
		FVector(200.0f, -6560.0f, 100.0f)
	};
	static const FVector WallTargets[] = {
		// Stand just outside the wall face; center-of-wall targets were unreachable.
		FVector(-1632.0f, -280.0f, 0.0f),
		FVector(1632.0f, 280.0f, 0.0f),
		FVector(200.0f, -1632.0f, 0.0f)
	};

	const int32 Lane = NextSpawnLane++ % UE_ARRAY_COUNT(SpawnLocations);
	const FTransform SpawnTransform(FRotator(0.0f, 0.0f, 0.0f), SpawnLocations[Lane]);
	ATDZombieCharacter* Zombie = GetWorld()->SpawnActor<ATDZombieCharacter>(ATDZombieCharacter::StaticClass(), SpawnTransform);
	if (Zombie)
	{
		Zombie->SetOutpostTarget(WallTargets[Lane]);
		ETDZombieArchetype Archetype = ETDZombieArchetype::Walker;
		const float VariantRoll = FMath::FRand();
		if (CurrentWave >= 4 && VariantRoll < 0.22f)
		{
			Archetype = ETDZombieArchetype::Brute;
		}
		else if (CurrentWave >= 2 && VariantRoll < 0.52f)
		{
			Archetype = ETDZombieArchetype::Runner;
		}
		Zombie->SetArchetype(Archetype, CurrentWave);
		ActiveZombies.Add(Zombie);
	}

	--ZombiesLeftToSpawn;
	GetWorldTimerManager().SetTimer(SpawnTimer, this, &AextractionFPSGameMode::SpawnNextZombie, 1.25f, false);
}

void AextractionFPSGameMode::NotifyZombieKilled(ATDZombieCharacter* Zombie)
{
	ActiveZombies.Remove(Zombie);
	Credits += 10;
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 1.5f, FColor(130, 255, 40),
			FString::Printf(TEXT("+10 CREDITS  |  %d available"), Credits));
	}
	CheckForWaveCompletion();
}

void AextractionFPSGameMode::DamageOutpost(float Damage)
{
	if (bOutpostOverrun || Damage <= 0.0f)
	{
		return;
	}
	OutpostHealth = FMath::Max(0.0f, OutpostHealth - Damage * OutpostDamageMultiplier);
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(41003, 2.0f, FColor::Red,
			FString::Printf(TEXT("OUTPOST HIT  |  Integrity: %d%%"), FMath::RoundToInt(OutpostHealth / 10.0f)));
	}
	if (OutpostHealth <= 0.0f)
	{
		bOutpostOverrun = true;
		GetWorldTimerManager().ClearTimer(WaveTimer);
		GetWorldTimerManager().ClearTimer(SpawnTimer);
		ZombiesLeftToSpawn = 0;
		ShowDefenseStatus(TEXT("OUTPOST OVERRUN  |  DEFENSE FAILED"), FColor::Red);
	}
}

bool AextractionFPSGameMode::UpgradeOutpostWalls()
{
	constexpr int32 Cost = 80;
	if (Credits < Cost || WallUpgradeLevel >= 3)
	{
		ShowDefenseStatus(WallUpgradeLevel >= 3 ? TEXT("Walls are fully reinforced") : TEXT("Wall reinforcement costs 80 credits"), FColor::Red);
		return false;
	}
	Credits -= Cost;
	++WallUpgradeLevel;
	OutpostDamageMultiplier = FMath::Max(0.45f, 1.0f - 0.18f * WallUpgradeLevel);
	ShowDefenseStatus(FString::Printf(TEXT("WALL REINFORCED %d/3 | %d credits"), WallUpgradeLevel, Credits), FColor(0, 230, 255));
	return true;
}

bool AextractionFPSGameMode::PurchaseDefense(ETDDefenseType Type, const FVector& Location, const FRotator& Rotation)
{
	const int32 Cost = Type == ETDDefenseType::Turret ? 60 : (Type == ETDDefenseType::GuardBot ? 90 : 35);
	if (Credits < Cost || !GetWorld())
	{
		ShowDefenseStatus(Credits < Cost ? TEXT("Not enough credits") : TEXT("Cannot place defense"), FColor::Red);
		return false;
	}
	FVector DeploymentLocation = Location;
	if (Type == ETDDefenseType::Turret)
	{
		// Turrets are built only on marked rooftop edges near all three zombie lanes.
		static const FVector RooftopSlots[] = {
			FVector(-1800.0f, 1400.0f, 904.0f),
			FVector(-1800.0f, -1400.0f, 904.0f),
			FVector(1800.0f, 1400.0f, 904.0f),
			FVector(1800.0f, -1400.0f, 904.0f),
			FVector(-1750.0f, -1600.0f, 904.0f),
			FVector(1750.0f, -1600.0f, 904.0f)
		};
		TArray<AActor*> ExistingDefenses;
		UGameplayStatics::GetAllActorsOfClass(this, ATDDefenseDeployable::StaticClass(), ExistingDefenses);
		float NearestDistanceSquared = TNumericLimits<float>::Max();
		for (const FVector& Slot : RooftopSlots)
		{
			bool bOccupied = false;
			for (const AActor* ExistingDefense : ExistingDefenses)
			{
				if (FVector::DistSquared2D(ExistingDefense->GetActorLocation(), Slot) < FMath::Square(300.0f))
				{
					bOccupied = true;
					break;
				}
			}
			if (bOccupied)
			{
				continue;
			}
			const float DistanceSquared = FVector::DistSquared2D(Location, Slot);
			if (DistanceSquared < NearestDistanceSquared)
			{
				NearestDistanceSquared = DistanceSquared;
				DeploymentLocation = Slot;
			}
		}
		if (NearestDistanceSquared == TNumericLimits<float>::Max())
		{
			ShowDefenseStatus(TEXT("All rooftop turret pads are occupied"), FColor::Red);
			return false;
		}
	}
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	ATDDefenseDeployable* Defense = GetWorld()->SpawnActor<ATDDefenseDeployable>(ATDDefenseDeployable::StaticClass(), DeploymentLocation, Rotation, SpawnParams);
	if (!Defense)
	{
		ShowDefenseStatus(TEXT("Placement failed"), FColor::Red);
		return false;
	}
	Defense->Configure(Type);
	Credits -= Cost;
	const TCHAR* DefenseName = Type == ETDDefenseType::Turret
		? TEXT("TURRET")
		: (Type == ETDDefenseType::GuardBot ? TEXT("GUARD BOT") : TEXT("BARBED WIRE"));
	ShowDefenseStatus(FString::Printf(TEXT("%s deployed | -%d credits | %d left"), DefenseName, Cost, Credits), FColor(120, 255, 80));
	return true;
}

bool AextractionFPSGameMode::UpgradeNearestTower(const FVector& Location)
{
	TArray<AActor*> Defenses;
	UGameplayStatics::GetAllActorsOfClass(this, ATDDefenseDeployable::StaticClass(), Defenses);
	ATDDefenseDeployable* NearestTower = nullptr;
	// Prefer the turret nearest the cursor, but allow the command to work even
	// when the mouse is outside the turret's interaction radius. This is a
	// keyboard-first TD control, so the player should not need pixel-perfect aim.
	float NearestDistanceSquared = TNumericLimits<float>::Max();
	for (AActor* Actor : Defenses)
	{
		ATDDefenseDeployable* Defense = Cast<ATDDefenseDeployable>(Actor);
		if (!Defense || !Defense->CanUpgrade())
		{
			continue;
		}
		const float DistanceSquared = FVector::DistSquared2D(Location, Defense->GetActorLocation());
		if (DistanceSquared < NearestDistanceSquared)
		{
			NearestDistanceSquared = DistanceSquared;
			NearestTower = Defense;
		}
	}

	if (!NearestTower)
	{
		ShowDefenseStatus(TEXT("Build a turret before upgrading"), FColor::Red);
		return false;
	}
	const int32 Cost = 50 * (NearestTower->GetUpgradeLevel() + 1);
	if (Credits < Cost)
	{
		ShowDefenseStatus(FString::Printf(TEXT("Tower upgrade costs %d credits"), Cost), FColor::Red);
		return false;
	}
	if (!NearestTower->Upgrade())
	{
		return false;
	}
	Credits -= Cost;
	ShowDefenseStatus(FString::Printf(TEXT("TURRET UPGRADE %d/3 | -%d credits | %d left"),
		NearestTower->GetUpgradeLevel(), Cost, Credits), FColor(255, 195, 50));
	return true;
}

bool AextractionFPSGameMode::RepairOutpost()
{
	constexpr int32 Cost = 40;
	if (Credits < Cost || OutpostHealth >= 1000.0f)
	{
		ShowDefenseStatus(OutpostHealth >= 1000.0f ? TEXT("Outpost is already at full integrity") : TEXT("Repair costs 40 credits"), FColor::Red);
		return false;
	}
	Credits -= Cost;
	OutpostHealth = FMath::Min(1000.0f, OutpostHealth + 300.0f);
	ShowDefenseStatus(FString::Printf(TEXT("Outpost repaired | %d credits left"), Credits), FColor(120, 255, 80));
	return true;
}

bool AextractionFPSGameMode::CallAirstrike(const FVector& Center)
{
	constexpr int32 Cost = 120;
	if (Credits < Cost)
	{
		ShowDefenseStatus(TEXT("Airstrike costs 120 credits"), FColor::Red);
		return false;
	}
	TArray<AActor*> Zombies;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ATDZombieCharacter::StaticClass(), Zombies);
	int32 Hits = 0;
	for (AActor* Actor : Zombies)
	{
		if (Actor && FVector::DistSquared2D(Actor->GetActorLocation(), Center) < FMath::Square(1800.0f))
		{
			Actor->TakeDamage(250.0f, FDamageEvent(), nullptr, this);
			++Hits;
		}
	}
	if (Hits == 0)
	{
		ShowDefenseStatus(TEXT("No zombies in the airstrike zone"), FColor::Red);
		return false;
	}
	Credits -= Cost;
	ShowDefenseStatus(FString::Printf(TEXT("EAGLE AIRSTRIKE | %d targets hit | %d credits"), Hits, Credits), FColor(0, 230, 255));
	return true;
}

bool AextractionFPSGameMode::RefillPlayerAmmo(APawn* PlayerPawn)
{
	constexpr int32 Cost = 25;
	AShooterCharacter* Shooter = Cast<AShooterCharacter>(PlayerPawn);
	if (!Shooter)
	{
		ShowDefenseStatus(TEXT("Ammo is only available to the armed player"), FColor::Red);
		return false;
	}
	if (Credits < Cost)
	{
		ShowDefenseStatus(TEXT("Ammo costs 25 credits"), FColor::Red);
		return false;
	}
	if (!Shooter->RefillCurrentWeapon())
	{
		ShowDefenseStatus(TEXT("No weapon equipped"), FColor::Red);
		return false;
	}
	Credits -= Cost;
	ShowDefenseStatus(FString::Printf(TEXT("Ammo refilled | %d credits"), Credits), FColor(120, 255, 80));
	return true;
}

void AextractionFPSGameMode::ShowDefenseStatus(const FString& Message, const FColor& Color) const
{
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.0f, Color, Message);
	}
}

void AextractionFPSGameMode::CheckForWaveCompletion()
{
	if (!bOutpostOverrun && ZombiesLeftToSpawn == 0 && ActiveZombies.IsEmpty())
	{
		GetWorldTimerManager().ClearTimer(SpawnTimer);
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 4.0f, FColor(60, 240, 255),
				TEXT("AREA CLEAR  |  10 seconds until the next wave"));
		}
		if (CurrentWave >= MaxWaves)
		{
			UnlockExtraction();
		}
		else
		{
			GetWorldTimerManager().SetTimer(WaveTimer, this, &AextractionFPSGameMode::StartNextWave, 10.0f, false);
		}
	}
}

void AextractionFPSGameMode::UnlockExtraction()
{
	if (bOutpostOverrun || bExtractionUnlocked)
	{
		return;
	}
	bExtractionUnlocked = true;

	for (ATDExtractionGate* Gate : ExtractionGates)
	{
		if (IsValid(Gate))
		{
			Gate->OpenGate();
		}
	}
	if (IsValid(ExtractionZone))
	{
		ExtractionZone->SetActive(true);
	}
	ShowDefenseStatus(TEXT("ALL 5 WAVES CLEARED  |  SOUTH EXIT OPEN — REACH THE HELICOPTER"), FColor(0, 230, 255));
}

void AextractionFPSGameMode::CompleteExtraction()
{
	if (!bExtractionUnlocked || bOutpostOverrun || bLevelComplete)
	{
		return;
	}
	bLevelComplete = true;
	GetWorldTimerManager().ClearTimer(WaveTimer);
	GetWorldTimerManager().ClearTimer(SpawnTimer);
	ShowDefenseStatus(TEXT("EXTRACTION COMPLETE  |  LEVEL 1 CLEARED"), FColor(120, 255, 80));
}

void AextractionFPSGameMode::RefreshDefenseHUD()
{
	if (!GEngine)
	{
		return;
	}
	const FString Status = FString::Printf(
		TEXT("WAVE %d/%d    CREDITS %d    OUTPOST %d%%    ZOMBIES %d"),
		CurrentWave, MaxWaves, Credits, FMath::RoundToInt(OutpostHealth / 10.0f),
		ActiveZombies.Num() + ZombiesLeftToSpawn);
	GEngine->AddOnScreenDebugMessage(41001, 0.6f, FColor(225, 250, 255), Status, false, FVector2D(1.25f, 1.25f));
	GEngine->AddOnScreenDebugMessage(41002, 0.6f, FColor(255, 190, 85),
		bExtractionUnlocked
			? TEXT("OBJECTIVE: Reach the helicopter outside town")
			: TEXT("BUY: 1 Turret 60 | 2 Bot 90 | 3 Wire 35 | 4 Repair 40 | 5 Strike 120 | 6 Upgrade 50+ | 7 Walls 80"),
		false, FVector2D(1.0f, 1.0f));
}
