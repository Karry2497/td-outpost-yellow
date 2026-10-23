// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TDDefenseProjectile.generated.h"

class UStaticMeshComponent;

UCLASS()
class EXTRACTIONFPS_API ATDDefenseProjectile : public AActor
{
	GENERATED_BODY()

public:
	ATDDefenseProjectile();
	void Configure(const FVector& Start, const FVector& Landing, float FlightSeconds, float ArcHeight,
		float Damage, float SplashRadius, bool bAsMine);

protected:
	virtual void Tick(float DeltaSeconds) override;

private:
	void Detonate();
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> ProjectileMesh;
	FVector StartLocation = FVector::ZeroVector;
	FVector LandingLocation = FVector::ZeroVector;
	float FlightDuration = 1.0f;
	float ArcHeightCm = 0.0f;
	float ElapsedFlight = 0.0f;
	float DamageAmount = 0.0f;
	float SplashRadiusCm = 0.0f;
	float TimeSinceLanding = 0.0f;
	float MineLifetime = 18.0f;
	bool bMine = false;
	bool bLanded = false;
	bool bArmed = false;
};
