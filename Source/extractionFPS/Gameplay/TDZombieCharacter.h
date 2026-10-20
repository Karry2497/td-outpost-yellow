// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "TDZombieCharacter.generated.h"

class UStaticMeshComponent;

UENUM(BlueprintType)
enum class ETDZombieArchetype : uint8
{
	Walker,
	Runner,
	Brute
};

/** Bright, readable zombie used by the first tower-defense wave prototype. */
UCLASS()
class EXTRACTIONFPS_API ATDZombieCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ATDZombieCharacter();

	void SetOutpostTarget(const FVector& TargetLocation);
	void SetArchetype(ETDZombieArchetype NewArchetype, int32 WaveNumber);

	virtual float TakeDamage(float Damage, struct FDamageEvent const& DamageEvent,
		AController* EventInstigator, AActor* DamageCauser) override;

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

private:
	void AttackOutpost();
	void Die();

	UPROPERTY(VisibleAnywhere, Category="Zombie|Style")
	TObjectPtr<UStaticMeshComponent> ChestHarness;

	UPROPERTY(VisibleAnywhere, Category="Zombie|Style")
	TObjectPtr<UStaticMeshComponent> ShoulderArmor;

	UPROPERTY(VisibleAnywhere, Category="Zombie|Style")
	TObjectPtr<UStaticMeshComponent> HazardStripe;

	UPROPERTY(EditAnywhere, Category="Zombie|Combat", meta=(ClampMin="1.0"))
	float Health = 70.0f;

	UPROPERTY(EditAnywhere, Category="Zombie|Combat", meta=(ClampMin="0.0"))
	float OutpostDamage = 18.0f;

	UPROPERTY(EditAnywhere, Category="Zombie|Combat", meta=(ClampMin="1.0"))
	float AttackRange = 255.0f;

	UPROPERTY(EditAnywhere, Category="Zombie|Movement", meta=(ClampMin="0.0"))
	float MoveSpeed = 285.0f;

	ETDZombieArchetype Archetype = ETDZombieArchetype::Walker;

	FVector OutpostTarget = FVector::ZeroVector;
	FTimerHandle AttackTimer;
	bool bIsDead = false;
};
