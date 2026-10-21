// Copyright Epic Games, Inc. All Rights Reserved.


#include "Variant_Shooter/ShooterPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "InputMappingContext.h"
#include "Camera/CameraActor.h"
#include "Camera/CameraComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"
#include "ShooterCharacter.h"
#include "ShooterBulletCounterUI.h"
#include "extractionFPS.h"
#include "Widgets/Input/SVirtualJoystick.h"
#include "extractionFPSGameMode.h"
#include "InputCoreTypes.h"
#include "Engine/World.h"

void AShooterPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// This project is now a tower-defense game: keep a fixed tactical camera on
	// the safe north side of the outpost and remove the old shooter pawn.
	bAutoManageActiveCameraTarget = false;
	if (GetWorld() && GetWorld()->GetAuthGameMode<AextractionFPSGameMode>())
	{
		if (APawn* StartingPawn = GetPawn())
		{
			if (AShooterCharacter* ShooterCharacter = Cast<AShooterCharacter>(StartingPawn))
			{
				ShooterCharacter->OnDestroyed.RemoveDynamic(this, &AShooterPlayerController::OnPawnDestroyed);
				ShooterCharacter->OnBulletCountUpdated.RemoveDynamic(this, &AShooterPlayerController::OnBulletCountUpdated);
				ShooterCharacter->OnDamaged.RemoveDynamic(this, &AShooterPlayerController::OnPawnDamaged);
			}
			UnPossess();
			StartingPawn->Destroy();
		}

		FActorSpawnParameters CameraSpawnParams;
		CameraSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		if (ACameraActor* DefenseCamera = GetWorld()->SpawnActor<ACameraActor>(
			ACameraActor::StaticClass(), FVector(0.0f, 10500.0f, 7600.0f), FRotator(-36.0f, -90.0f, 0.0f), CameraSpawnParams))
		{
			UCameraComponent* CameraComponent = DefenseCamera->GetCameraComponent();
			CameraComponent->SetProjectionMode(ECameraProjectionMode::Perspective);
			CameraComponent->FieldOfView = 94.0f;
			SetViewTarget(DefenseCamera);
		}
	}

	// TD builds are issued against the world point under the mouse cursor.
	bShowMouseCursor = true;
	bEnableClickEvents = true;
	bEnableMouseOverEvents = true;
}

void AShooterPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// only add IMCs for local player controllers
	if (IsLocalPlayerController())
	{
		InputComponent->BindKey(EKeys::One, IE_Pressed, this, &AShooterPlayerController::DefenseHotkey1);
		InputComponent->BindKey(EKeys::Two, IE_Pressed, this, &AShooterPlayerController::DefenseHotkey2);
		InputComponent->BindKey(EKeys::Three, IE_Pressed, this, &AShooterPlayerController::DefenseHotkey3);
		InputComponent->BindKey(EKeys::Four, IE_Pressed, this, &AShooterPlayerController::DefenseHotkey4);
		InputComponent->BindKey(EKeys::Five, IE_Pressed, this, &AShooterPlayerController::DefenseHotkey5);
		InputComponent->BindKey(EKeys::Six, IE_Pressed, this, &AShooterPlayerController::DefenseHotkey6);
		InputComponent->BindKey(EKeys::Seven, IE_Pressed, this, &AShooterPlayerController::DefenseHotkey7);
		// add the input mapping contexts
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
		{
			for (UInputMappingContext* CurrentContext : DefaultMappingContexts)
			{
				Subsystem->AddMappingContext(CurrentContext, 0);
			}

			// only add these IMCs if we're not using mobile touch input
			if (!ShouldUseTouchControls())
			{
				for (UInputMappingContext* CurrentContext : MobileExcludedMappingContexts)
				{
					Subsystem->AddMappingContext(CurrentContext, 0);
				}
			}
		}

		if (ShouldUseTouchControls())
		{
			// spawn the mobile controls widget
			MobileControlsWidget = CreateWidget<UUserWidget>(this, MobileControlsWidgetClass);

			if (MobileControlsWidget)
			{
				// add the controls to the player screen
				MobileControlsWidget->AddToPlayerScreen(0);

			} else {

				UE_LOG(LogextractionFPS, Error, TEXT("Could not spawn mobile controls widget."));

			}
		}

	}
}

void AShooterPlayerController::UseDefenseHotkey(uint8 Hotkey)
{
	AextractionFPSGameMode* GameMode = GetWorld() ? GetWorld()->GetAuthGameMode<AextractionFPSGameMode>() : nullptr;
	APawn* PlayerPawn = GetPawn();
	if (!GameMode)
	{
		return;
	}
	const FVector Forward = PlayerPawn ? PlayerPawn->GetActorForwardVector() : GetControlRotation().Vector();
	const FVector PlaceLocation = GetDefensePlacementLocation();
	switch (Hotkey)
	{
	case 1: GameMode->PurchaseDefense(ETDDefenseType::Turret, PlaceLocation, Forward.Rotation()); break;
	case 2: GameMode->PurchaseDefense(ETDDefenseType::GuardBot, PlaceLocation, Forward.Rotation()); break;
	case 3: GameMode->PurchaseDefense(ETDDefenseType::BarbedWire, PlaceLocation, Forward.Rotation()); break;
	case 4: GameMode->RepairOutpost(); break;
	case 5: GameMode->CallAirstrike(PlaceLocation); break;
	case 6: GameMode->UpgradeNearestTower(PlaceLocation); break;
	case 7: GameMode->UpgradeOutpostWalls(); break;
	default: break;
	}
}

FVector AShooterPlayerController::GetDefensePlacementLocation() const
{
	FHitResult CursorHit;
	if (GetHitResultUnderCursor(ECC_Visibility, false, CursorHit))
	{
		return CursorHit.ImpactPoint + FVector(0.0f, 0.0f, 8.0f);
	}

	FVector ViewLocation;
	FRotator ViewRotation;
	GetPlayerViewPoint(ViewLocation, ViewRotation);
	const FVector DesiredXY = ViewLocation + ViewRotation.Vector() * 1200.0f;
	const FVector TraceStart(DesiredXY.X, DesiredXY.Y, ViewLocation.Z + 1000.0f);
	const FVector TraceEnd(DesiredXY.X, DesiredXY.Y, ViewLocation.Z - 5000.0f);
	FHitResult GroundHit;
	FCollisionQueryParams Params(SCENE_QUERY_STAT(DefensePlacementGround), false, GetPawn());
	if (GetWorld() && GetWorld()->LineTraceSingleByChannel(GroundHit, TraceStart, TraceEnd, ECC_Visibility, Params))
	{
		return GroundHit.ImpactPoint + FVector(0.0f, 0.0f, 8.0f);
	}
	if (APawn* PlayerPawn = GetPawn())
	{
		return PlayerPawn->GetActorLocation() + PlayerPawn->GetActorForwardVector() * 420.0f
			- FVector(0.0f, 0.0f, 70.0f);
	}
	const FVector FlatForward = FVector(ViewRotation.Vector().X, ViewRotation.Vector().Y, 0.0f).GetSafeNormal();
	return ViewLocation + FlatForward * 1200.0f;
}

void AShooterPlayerController::DefenseHotkey1() { UseDefenseHotkey(1); }
void AShooterPlayerController::DefenseHotkey2() { UseDefenseHotkey(2); }
void AShooterPlayerController::DefenseHotkey3() { UseDefenseHotkey(3); }
void AShooterPlayerController::DefenseHotkey4() { UseDefenseHotkey(4); }
void AShooterPlayerController::DefenseHotkey5() { UseDefenseHotkey(5); }
void AShooterPlayerController::DefenseHotkey6() { UseDefenseHotkey(6); }
void AShooterPlayerController::DefenseHotkey7() { UseDefenseHotkey(7); }

void AShooterPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	// subscribe to the pawn's OnDestroyed delegate
	InPawn->OnDestroyed.AddDynamic(this, &AShooterPlayerController::OnPawnDestroyed);

	// is this a shooter character?
	if (AShooterCharacter* ShooterCharacter = Cast<AShooterCharacter>(InPawn))
	{
		// add the player tag
		ShooterCharacter->Tags.Add(PlayerPawnTag);

		// set the team
		ShooterCharacter->SetTeam(TeamByte);

		// subscribe to the pawn's delegates
		ShooterCharacter->OnBulletCountUpdated.AddDynamic(this, &AShooterPlayerController::OnBulletCountUpdated);
		ShooterCharacter->OnDamaged.AddDynamic(this, &AShooterPlayerController::OnPawnDamaged);

		// force update the life bar
		ShooterCharacter->OnDamaged.Broadcast(1.0f);
	}
}

void AShooterPlayerController::OnPawnDestroyed(AActor* DestroyedActor)
{
	// reset the bullet counter HUD
	if (IsValid(BulletCounterUI))
	{
		BulletCounterUI->BP_UpdateBulletCounter(0, 0);
	}

	if (TeamByte < TeamTags.Num())
	{
		// find the player start
		TArray<AActor*> ActorList;
		UGameplayStatics::GetAllActorsOfClassWithTag(GetWorld(), APlayerStart::StaticClass(), TeamTags[TeamByte], ActorList);

		if (ActorList.Num() > 0)
		{
			// select a random player start
			AActor* RandomPlayerStart = ActorList[FMath::RandRange(0, ActorList.Num() - 1)];

			// spawn a character at the player start
			const FTransform SpawnTransform = RandomPlayerStart->GetActorTransform();

			if (AShooterCharacter* RespawnedCharacter = GetWorld()->SpawnActor<AShooterCharacter>(CharacterClass, SpawnTransform))
			{
				// possess the character
				Possess(RespawnedCharacter);
			}
		}
	}
}

void AShooterPlayerController::OnBulletCountUpdated(int32 MagazineSize, int32 Bullets)
{
	// update the UI
	if (BulletCounterUI)
	{
		BulletCounterUI->BP_UpdateBulletCounter(MagazineSize, Bullets);
	}
}

void AShooterPlayerController::OnPawnDamaged(float LifePercent)
{
	if (IsValid(BulletCounterUI))
	{
		BulletCounterUI->BP_Damaged(LifePercent);
	}
}

bool AShooterPlayerController::ShouldUseTouchControls() const
{
	// are we on a mobile platform? Should we force touch?
	return SVirtualJoystick::ShouldDisplayTouchInterface() || bForceTouchControls;
}

void AShooterPlayerController::SetTeam(uint8 Team)
{
	TeamByte = Team;

	// if we already have a pawn, set its team
	if (AShooterCharacter* ShooterCharacter = Cast<AShooterCharacter>(GetPawn()))
	{
		ShooterCharacter->SetTeam(Team);
	}
}
