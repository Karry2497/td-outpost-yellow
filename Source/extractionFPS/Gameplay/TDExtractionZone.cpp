// Copyright Epic Games, Inc. All Rights Reserved.

#include "Gameplay/TDExtractionZone.h"

#include "Components/BoxComponent.h"
#include "GameFramework/Pawn.h"
#include "extractionFPSGameMode.h"

ATDExtractionZone::ATDExtractionZone()
{
	PrimaryActorTick.bCanEverTick = false;
	Trigger = CreateDefaultSubobject<UBoxComponent>(TEXT("Extraction Trigger"));
	SetRootComponent(Trigger);
	Trigger->SetBoxExtent(FVector(500.0f, 500.0f, 250.0f));
	Trigger->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Trigger->SetCollisionResponseToAllChannels(ECR_Ignore);
	Trigger->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	Trigger->SetGenerateOverlapEvents(true);
	Trigger->OnComponentBeginOverlap.AddDynamic(this, &ATDExtractionZone::OnPlayerEntered);
}

void ATDExtractionZone::SetActive(bool bNewActive)
{
	Trigger->SetCollisionEnabled(bNewActive ? ECollisionEnabled::QueryOnly : ECollisionEnabled::NoCollision);
}

void ATDExtractionZone::OnPlayerEntered(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!Cast<APawn>(OtherActor) || !GetWorld())
	{
		return;
	}
	if (AextractionFPSGameMode* GameMode = GetWorld()->GetAuthGameMode<AextractionFPSGameMode>())
	{
		GameMode->CompleteExtraction();
	}
}
