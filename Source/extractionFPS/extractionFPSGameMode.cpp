// Copyright Epic Games, Inc. All Rights Reserved.

#include "extractionFPSGameMode.h"
#include "Gameplay/TDZombieCharacter.h"
#include "Gameplay/TDDefenseDeployable.h"
#include "Gameplay/TDExtractionGate.h"
#include "Gameplay/TDExtractionZone.h"
#include "Gameplay/TDGameUI.h"
#include "Variant_Shooter/ShooterPlayerController.h"
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

	GetWorldTimerManager().SetTimer(HUDTimer, this, &AextractionFPSGameMode::RefreshDefenseHUD, 0.5f, true, 0.0f);
}

void AextractionFPSGameMode::StartDefense()
{
	if (bGameStarted)
	{
		ShowDefenseStatus(TEXT("DEFENSE RESUMED"), FColor(120, 255, 80));
		return;
	}
	bGameStarted = true;
	ShowDefenseStatus(TEXT("PREPARE FOR INCOMING WAVES"), FColor(0, 230, 255));
	GetWorldTimerManager().SetTimer(WaveTimer, this, &AextractionFPSGameMode::StartNextWave, 5.0f, false);
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
	if (!bGameStarted || bOutpostOverrun || bLevelComplete || bExtractionUnlocked || CurrentWave >= MaxWaves)
	{
		return;
	}
	++CurrentWave;
	ZombiesLeftToSpawn = 8 + CurrentWave * 4;

	ShowDefenseStatus(FString::Printf(TEXT("WAVE %d INBOUND  •  %d ZOMBIES"), CurrentWave, ZombiesLeftToSpawn), FColor(255, 220, 0));

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
		static const FName ZombieLaneTags[] = {
			FName(TEXT("ZombieLane_West")),
			FName(TEXT("ZombieLane_East")),
			FName(TEXT("ZombieLane_South"))
		};
		Zombie->Tags.Add(ZombieLaneTags[Lane]);
		Zombie->SetOutpostTarget(WallTargets[Lane]);
		ETDZombieArchetype Archetype = ETDZombieArchetype::Walker;
		const float VariantRoll = FMath::FRand();
		if (CurrentWave >= 4 && VariantRoll < 0.16f)
		{
			Archetype = ETDZombieArchetype::Exploder;
		}
		else if (CurrentWave >= 3 && VariantRoll < 0.32f)
		{
			Archetype = ETDZombieArchetype::Brute;
		}
		else if (CurrentWave >= 2 && VariantRoll < 0.52f)
		{
			Archetype = ETDZombieArchetype::RadioactiveRunner;
		}
		else if (VariantRoll < 0.76f)
		{
			Archetype = ETDZombieArchetype::Crawler;
		}
		Zombie->SetArchetype(Archetype, CurrentWave);
		ActiveZombies.Add(Zombie);
	}

	--ZombiesLeftToSpawn;
	GetWorldTimerManager().SetTimer(SpawnTimer, this, &AextractionFPSGameMode::SpawnNextZombie, 0.8f, false);
}

void AextractionFPSGameMode::NotifyZombieKilled(ATDZombieCharacter* Zombie, bool bAwardCredits)
{
	ActiveZombies.Remove(Zombie);
	if (bAwardCredits)
	{
		Credits += 10;
		ShowDefenseStatus(FString::Printf(TEXT("ZOMBIE ELIMINATED  •  +10 CREDITS  •  %d AVAILABLE"), Credits), FColor(130, 255, 40));
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
	ShowDefenseStatus(FString::Printf(TEXT("OUTPOST HIT  •  INTEGRITY %d%%"), FMath::RoundToInt(OutpostHealth / 10.0f)), FColor::Red);
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
		const TArray<FVector> AvailableSlots = GetAvailableTurretSlots();
		float NearestDistanceSquared = TNumericLimits<float>::Max();
		for (const FVector& Slot : AvailableSlots)
		{
			const float DistanceSquared = FVector::DistSquared2D(Location, Slot);
			if (DistanceSquared < NearestDistanceSquared)
			{
				NearestDistanceSquared = DistanceSquared;
				DeploymentLocation = Slot;
			}
		}
		if (NearestDistanceSquared > FMath::Square(250.0f))
		{
			ShowDefenseStatus(AvailableSlots.IsEmpty() ? TEXT("All rooftop turret pads are occupied") : TEXT("Click one of the marked rooftop pads"), FColor::Red);
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

TArray<FVector> AextractionFPSGameMode::GetAvailableTurretSlots() const
{
	// These world-space positions mirror the builder's twelve pads after its 0.8 scale.
	static const FVector RooftopSlots[] = {
		FVector(-1840.0f, 1840.0f, 912.0f), FVector(-4000.0f, 1840.0f, 912.0f), FVector(-1840.0f, 4000.0f, 912.0f),
		FVector(1840.0f, 1840.0f, 912.0f), FVector(4000.0f, 1840.0f, 912.0f), FVector(1840.0f, 4000.0f, 912.0f),
		FVector(-1840.0f, -1840.0f, 912.0f), FVector(-4000.0f, -1840.0f, 912.0f), FVector(-1840.0f, -4000.0f, 912.0f),
		FVector(1840.0f, -1840.0f, 912.0f), FVector(4000.0f, -1840.0f, 912.0f), FVector(1840.0f, -4000.0f, 912.0f)
	};
	TArray<AActor*> ExistingDefenses;
	UGameplayStatics::GetAllActorsOfClass(this, ATDDefenseDeployable::StaticClass(), ExistingDefenses);
	TArray<FVector> AvailableSlots;
	for (const FVector& Slot : RooftopSlots)
	{
		const bool bOccupied = ExistingDefenses.ContainsByPredicate([&Slot](const AActor* ExistingDefense)
		{
			const ATDDefenseDeployable* ExistingTurret = Cast<ATDDefenseDeployable>(ExistingDefense);
			return IsValid(ExistingTurret) && ExistingTurret->IsTurret()
				&& FVector::DistSquared2D(ExistingDefense->GetActorLocation(), Slot) < FMath::Square(300.0f);
		});
		if (!bOccupied)
		{
			AvailableSlots.Add(Slot);
		}
	}
	return AvailableSlots;
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

bool AextractionFPSGameMode::CallAirstrike(uint8 LaneIndex)
{
	constexpr int32 Cost = 240;
	if (Credits < Cost)
	{
		ShowDefenseStatus(TEXT("Airstrike costs 240 credits"), FColor::Red);
		return false;
	}
	static const FName ZombieLaneTags[] = {
		FName(TEXT("ZombieLane_West")),
		FName(TEXT("ZombieLane_East")),
		FName(TEXT("ZombieLane_South"))
	};
	if (LaneIndex >= UE_ARRAY_COUNT(ZombieLaneTags))
	{
		ShowDefenseStatus(TEXT("Invalid airstrike lane"), FColor::Red);
		return false;
	}
	TArray<AActor*> Zombies;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ATDZombieCharacter::StaticClass(), Zombies);
	int32 Hits = 0;
	for (AActor* Actor : Zombies)
	{
		if (Actor && Actor->Tags.Contains(ZombieLaneTags[LaneIndex]))
		{
			Actor->TakeDamage(250.0f, FDamageEvent(), nullptr, this);
			++Hits;
		}
	}
	if (Hits == 0)
	{
		static const TCHAR* LaneNames[] = { TEXT("west"), TEXT("east"), TEXT("south") };
		ShowDefenseStatus(FString::Printf(TEXT("No zombies on the %s approach"), LaneNames[LaneIndex]), FColor::Red);
		return false;
	}
	Credits -= Cost;
	static const TCHAR* LaneNames[] = { TEXT("WEST"), TEXT("EAST"), TEXT("SOUTH") };
	ShowDefenseStatus(FString::Printf(TEXT("EAGLE AIRSTRIKE | %s LANE | %d targets hit | %d credits"), LaneNames[LaneIndex], Hits, Credits), FColor(0, 230, 255));
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

void AextractionFPSGameMode::ShowDefenseStatus(const FString& Message, const FColor& Color)
{
	LastDefenseStatus = Message;
	if (AShooterPlayerController* PC = Cast<AShooterPlayerController>(UGameplayStatics::GetPlayerController(this, 0))) PC->UpdateTDHUD();
}

void AextractionFPSGameMode::CheckForWaveCompletion()
{
	if (!bOutpostOverrun && ZombiesLeftToSpawn == 0 && ActiveZombies.IsEmpty())
	{
		GetWorldTimerManager().ClearTimer(SpawnTimer);
		ShowDefenseStatus(TEXT("AREA CLEAR  •  NEXT WAVE IN 10 SECONDS"), FColor(60, 240, 255));
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
	if (AShooterPlayerController* PC = Cast<AShooterPlayerController>(UGameplayStatics::GetPlayerController(this, 0))) PC->UpdateTDHUD();
}
