// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TDGameUI.generated.h"

class UTextBlock;
class UProgressBar;
class UCanvasPanel;

UCLASS()
class EXTRACTIONFPS_API UTDGameUI : public UUserWidget
{
	GENERATED_BODY()

public:
	void ShowMainMenu();
	void ShowHelp();
	void ShowCommandMenu();
	void ShowTurretShop();
	void ShowPauseMenu();
	void ShowAirstrikeTargetMenu();
	void ShowHUD();
	void RefreshDisplay();

private:
	enum class EPage : uint8 { Main, Help, Command, TurretShop, Pause, Airstrike, HUD };
	EPage CurrentPage = EPage::Main;
	UPROPERTY()
	TObjectPtr<UCanvasPanel> RootCanvas;
	TObjectPtr<UTextBlock> WaveText;
	TObjectPtr<UTextBlock> CreditsText;
	TObjectPtr<UTextBlock> ZombieText;
	TObjectPtr<UTextBlock> HealthText;
	TObjectPtr<UTextBlock> CameraText;
	TObjectPtr<UTextBlock> StatusText;
	TObjectPtr<UTextBlock> ControlHintText;
	TObjectPtr<UProgressBar> HealthBar;
	UPROPERTY()
	TArray<TObjectPtr<UTextBlock>> TurretPlacementArrows;

	void BuildPage();
	void AddCommandButton(class UVerticalBox* List, int32 Number, const FString& Name, const FString& Cost, const FString& Description);
	class UButton* AddButton(class UVerticalBox* List, const FString& Label, const FLinearColor& Color);
	UTextBlock* AddLabel(class UVerticalBox* List, const FString& Label, int32 FontSize, const FLinearColor& Color, bool bBold = false);

	UFUNCTION() void StartClicked();
	UFUNCTION() void HelpClicked();
	UFUNCTION() void BackClicked();
	UFUNCTION() void ResumeClicked();
	UFUNCTION() void CommandClicked();
	UFUNCTION() void RestartClicked();
	UFUNCTION() void MainMenuClicked();
	UFUNCTION() void ExitClicked();
	UFUNCTION() void Action1Clicked();
	UFUNCTION() void Action2Clicked();
	UFUNCTION() void Action3Clicked();
	UFUNCTION() void Action4Clicked();
	UFUNCTION() void Action5Clicked();
	UFUNCTION() void Action6Clicked();
	UFUNCTION() void Action7Clicked();
	UFUNCTION() void TurretMediumClicked();
	UFUNCTION() void TurretHeavyClicked();
	UFUNCTION() void TurretMineClicked();
	UFUNCTION() void TurretArtilleryClicked();
	UFUNCTION() void AirstrikeWestClicked();
	UFUNCTION() void AirstrikeEastClicked();
	UFUNCTION() void AirstrikeSouthClicked();
	UFUNCTION() void AirstrikeCancelClicked();
	void DoAction(uint8 Number);
};
