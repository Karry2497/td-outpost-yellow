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
	if (bOutpostOverrun)
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
		FVector(-8600.0f, -350.0f, 100.0f),
		FVector(8600.0f, 350.0f, 100.0f),
		FVector(250.0f, -8200.0f, 100.0f)
	};
	static const FVector WallTargets[] = {
		// Stand just outside the wall face; center-of-wall targets were unreachable.
		FVector(-2040.0f, -350.0f, 0.0f),
		FVector(2040.0f, 350.0f, 0.0f),
		FVector(250.0f, -2040.0f, 0.0f)
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
		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red,
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
	const TCHAR* DefenseName = Type == ETDDefenseType::Turret
		? TEXT("TURRET")
		: (Type == ETDDefenseType::GuardBot ? TEXT("GUARD BOT") : TEXT("BARBED WIRE"));
	ShowDefenseStatus(FString::Printf(TEXT("%s deployed | -%d credits | %d left"), DefenseName, Cost, Credits), FColor(120, 255, 80));
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
		GetWorldTimerManager().SetTimer(WaveTimer, this, &AextractionFPSGameMode::StartNextWave, 10.0f, false);
	}
}

void AextractionFPSGameMode::RefreshDefenseHUD()
{
	if (!GEngine)
	{
		return;
	}
	const FString Status = FString::Printf(
		TEXT("WAVE %d    CREDITS %d    OUTPOST %d%%    ZOMBIES %d"),
		CurrentWave, Credits, FMath::RoundToInt(OutpostHealth / 10.0f),
		ActiveZombies.Num() + ZombiesLeftToSpawn);
	GEngine->AddOnScreenDebugMessage(41001, 0.6f, FColor(225, 250, 255), Status, false, FVector2D(1.25f, 1.25f));
	GEngine->AddOnScreenDebugMessage(41002, 0.6f, FColor(255, 190, 85),
		TEXT("BUY: 1 Turret 60 | 2 Bot 90 | 3 Wire 35 | 4 Repair 40 | 5 Strike 120 | 6 Ammo 25 | 7 Walls 80"),
		false, FVector2D(1.0f, 1.0f));
}
