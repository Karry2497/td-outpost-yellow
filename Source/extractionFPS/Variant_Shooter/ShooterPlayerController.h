// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "ShooterPlayerController.generated.h"

class UInputMappingContext;
class ATDTurretPlacementMarker;
class AShooterCharacter;
class UShooterBulletCounterUI;
class UTDGameUI;
class ACameraActor;
enum class ETDDefenseType : uint8;

/**
 *  Simple PlayerController for a first person shooter game
 *  Manages input mappings
 *  Respawns the player pawn when it's destroyed
 */
UCLASS(abstract, config="Game")
class EXTRACTIONFPS_API AShooterPlayerController : public APlayerController
{
	GENERATED_BODY()
	
protected:
	virtual void Tick(float DeltaSeconds) override;

	/** Input mapping contexts for this player */
	UPROPERTY(EditAnywhere, Category="Input|Input Mappings")
	TArray<UInputMappingContext*> DefaultMappingContexts;

	/** Input Mapping Contexts */
	UPROPERTY(EditAnywhere, Category="Input|Input Mappings")
	TArray<UInputMappingContext*> MobileExcludedMappingContexts;

	/** Mobile controls widget to spawn */
	UPROPERTY(EditAnywhere, Category="Input|Touch Controls")
	TSubclassOf<UUserWidget> MobileControlsWidgetClass;

	/** Pointer to the mobile controls widget */
	UPROPERTY()
	TObjectPtr<UUserWidget> MobileControlsWidget;

	/** If true, the player will use UMG touch controls even if not playing on mobile platforms */
	UPROPERTY(EditAnywhere, Config, Category = "Input|Touch Controls")
	bool bForceTouchControls = false;

	/** Character class to respawn when the possessed pawn is destroyed */
	UPROPERTY(EditAnywhere, Category="Shooter|Respawn")
	TSubclassOf<AShooterCharacter> CharacterClass;

	/** Type of bullet counter UI widget to spawn */
	UPROPERTY(EditAnywhere, Category="Shooter|UI")
	TSubclassOf<UShooterBulletCounterUI> BulletCounterUIClass;

	/** Tag to grant the possessed pawn to flag it as the player */
	UPROPERTY(EditAnywhere, Category="Shooter|Player")
	FName PlayerPawnTag = FName("Player");

	/** Pointer to the bullet counter UI widget */
	UPROPERTY()
	TObjectPtr<UShooterBulletCounterUI> BulletCounterUI;

	/** Team ID for this player */
	uint8 TeamByte = 0;

	UPROPERTY()
	TObjectPtr<UTDGameUI> TDGameUI;
	bool bHelpReturnsToMain = false;

	/** Tags that identify each team for PlayerStart selection upon respawning */
	UPROPERTY(EditAnywhere, Category="Shooter|Team")
	TArray<FName> TeamTags;

protected:

	/** Gameplay Initialization */
	virtual void BeginPlay() override;

	/** Initialize input bindings */
	virtual void SetupInputComponent() override;

	/** Pawn initialization */
	virtual void OnPossess(APawn* InPawn) override;

	/** Called if the possessed pawn is destroyed */
	UFUNCTION()
	void OnPawnDestroyed(AActor* DestroyedActor);

	/** Called when the bullet count on the possessed pawn is updated */
	UFUNCTION()
	void OnBulletCountUpdated(int32 MagazineSize, int32 Bullets);

	/** Called when the possessed pawn is damaged */
	UFUNCTION()
	void OnPawnDamaged(float LifePercent);

	/** Returns true if the player should use UMG touch controls */
	bool ShouldUseTouchControls() const;
	void UseDefenseHotkey(uint8 Hotkey);
	void BeginTurretPlacement();
	void ConfirmTurretPlacement();
	void CancelTurretPlacement();
	void ClearTurretPlacementMarkers();
	FVector GetDefensePlacementLocation() const;
	void DefenseHotkey1();
	void DefenseHotkey2();
	void DefenseHotkey3();
	void DefenseHotkey4();
	void DefenseHotkey5();
	void DefenseHotkey6();
	void DefenseHotkey7();
	void AirstrikeLane1();
	void AirstrikeLane2();
	void AirstrikeLane3();
	void CameraNorth();
	void CameraSouth();
	void CameraWest();
	void CameraEast();
	void CameraOutpost();
	void MoveToCameraStop(uint8 StopIndex);
	void UpdateCameraStopLabel();
	FTransform GetCameraStopTransform(uint8 StopIndex) const;
	void TurretPlacementClick();
	void TurretPlacementCancelClick();
	void BeginAirstrikeTargetSelection();

public:
	AShooterPlayerController();
	FString GetCameraStopName() const;
	uint8 GetCameraStopIndex() const { return CurrentCameraStop; }
	void StartDefense();
	void ExecuteDefenseAction(uint8 Hotkey);
	void OpenTurretShop();
	void BeginTurretPlacement(ETDDefenseType Type);
	void ToggleCommandMenu();
	void TogglePauseMenu();
	void ShowMainMenu();
	void ShowHowTo();
	void ReturnToCurrentMenu();
	void CloseOverlayAndResume();
	void RestartLevel();
	void UpdateTDHUD();
	void ConfirmAirstrikeTarget(uint8 LaneIndex);
	void CancelAirstrikeTarget();

	/** Assigns a team ID to this player */
	void SetTeam(uint8 Team);

private:
	UPROPERTY()
	TObjectPtr<ACameraActor> DefenseCamera;
	UPROPERTY()
	TArray<TObjectPtr<ATDTurretPlacementMarker>> TurretPlacementMarkers;
	TArray<FVector> TurretPlacementLocations;
	ETDDefenseType PendingTurretType;
	uint8 CurrentCameraStop = 0;
	bool bCameraTransitioning = false;
	bool bTurretPlacementPending = false;
	bool bAirstrikeTargetPending = false;
	float CameraTransitionElapsed = 0.0f;
	float CameraTransitionDuration = 0.85f;
	FTransform CameraTransitionStart;
	FTransform CameraTransitionTarget;

public:
	bool IsTurretPlacementPending() const { return bTurretPlacementPending; }
	bool IsAirstrikeTargetPending() const { return bAirstrikeTargetPending; }
	void GetTurretPlacementScreenPositions(TArray<FVector2D>& OutPositions) const;
};
