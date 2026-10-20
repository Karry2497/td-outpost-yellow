// Copyright Epic Games, Inc. All Rights Reserved.

#include "extractionFPSGameMode.h"
#include "Gameplay/TDZombieCharacter.h"
#include "Gameplay/TDDefenseDeployable.h"
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

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 6.0f, FColor::Yellow,
			TEXT("OUTPOST DEFENSE  |  First wave in 8s  |  1 Turret  2 Bot  3 Wire  4 Repair  5 Airstrike  6 Ammo  7 Walls"));
	}

	GetWorldTimerManager().SetTimer(WaveTimer, this, &AextractionFPSGameMode::StartNextWave, 8.0f, false);
}

void AextractionFPSGameMode::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GetWorldTimerManager().ClearTimer(WaveTimer);
	GetWorldTimerManager().ClearTimer(SpawnTimer);
	Super::EndPlay(EndPlayReason);
}

void AextractionFPSGameMode::StartNextWave()
{
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
		FVector(-8600.0f, -350.0f, 100.0f),
		FVector(8600.0f, 350.0f, 100.0f),
		FVector(250.0f, -8200.0f, 100.0f)
	};
	static const FVector WallTargets[] = {
		FVector(-1500.0f, -350.0f, 0.0f),
		FVector(1500.0f, 350.0f, 0.0f),
		FVector(250.0f, -1500.0f, 0.0f)
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
	OutpostHealth = FMath::Max(0.0f, OutpostHealth - Damage * OutpostDamageMultiplier);
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red,
			FString::Printf(TEXT("OUTPOST HIT  |  Integrity: %d%%"), FMath::RoundToInt(OutpostHealth / 10.0f)));
	}
	if (OutpostHealth <= 0.0f && GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Red, TEXT("OUTPOST OVERRUN  |  DEFENSE FAILED"));
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
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	ATDDefenseDeployable* Defense = GetWorld()->SpawnActor<ATDDefenseDeployable>(ATDDefenseDeployable::StaticClass(), Location, Rotation, SpawnParams);
	if (!Defense)
	{
		ShowDefenseStatus(TEXT("Placement failed"), FColor::Red);
		return false;
	}
	Defense->Configure(Type);
	Credits -= Cost;
	ShowDefenseStatus(FString::Printf(TEXT("Defense deployed (-%d) | %d credits"), Cost, Credits), FColor(120, 255, 80));
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
	Credits -= Cost;
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
	ShowDefenseStatus(FString::Printf(TEXT("EAGLE AIRSTRIKE | %d targets hit | %d credits"), Hits, Credits), FColor(0, 230, 255));
	return true;
}

bool AextractionFPSGameMode::RefillPlayerAmmo(APawn* PlayerPawn)
{
	constexpr int32 Cost = 25;
	AShooterCharacter* Shooter = Cast<AShooterCharacter>(PlayerPawn);
	if (!Shooter || Credits < Cost || !Shooter->RefillCurrentWeapon())
	{
		ShowDefenseStatus(TEXT("Ammo refill costs 25 credits"), FColor::Red);
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
	if (ZombiesLeftToSpawn == 0 && ActiveZombies.IsEmpty() && OutpostHealth > 0.0f)
	{
		GetWorldTimerManager().ClearTimer(SpawnTimer);
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 4.0f, FColor(60, 240, 255),
				TEXT("AREA CLEAR  |  10 seconds until the next wave"));
		}
		GetWorldTimerManager().SetTimer(WaveTimer, this, &AextractionFPSGameMode::StartNextWave, 10.0f, false);
	}
}
