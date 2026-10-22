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
	void ShowPauseMenu();
	void ShowHUD();
	void RefreshDisplay();

private:
	enum class EPage : uint8 { Main, Help, Command, Pause, HUD };
	EPage CurrentPage = EPage::Main;
	UPROPERTY()
	TObjectPtr<UCanvasPanel> RootCanvas;
	TObjectPtr<UTextBlock> WaveText;
	TObjectPtr<UTextBlock> CreditsText;
	TObjectPtr<UTextBlock> ZombieText;
	TObjectPtr<UTextBlock> HealthText;
	TObjectPtr<UTextBlock> StatusText;
	TObjectPtr<UProgressBar> HealthBar;

	void BuildPage();
	void AddCommandButton(class UVerticalBox* List, int32 Number, const FString& Name, const FString& Cost, const FString& Description);
	class UButton* AddButton(class UVerticalBox* List, const FString& Label, const FLinearColor& Color);
	void AddLabel(class UVerticalBox* List, const FString& Label, int32 FontSize, const FLinearColor& Color, bool bBold = false);

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
	void DoAction(uint8 Number);
};
