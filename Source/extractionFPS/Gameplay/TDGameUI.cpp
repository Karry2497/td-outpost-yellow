// Copyright Epic Games, Inc. All Rights Reserved.

#include "Gameplay/TDGameUI.h"
#include "Blueprint/WidgetTree.h"
#include "Blueprint/WidgetLayoutLibrary.h"

#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/ProgressBar.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Styling/CoreStyle.h"
#include "extractionFPSGameMode.h"
#include "Variant_Shooter/ShooterPlayerController.h"

namespace TDUIStyle
{
	const FLinearColor Ink(0.025f, 0.055f, 0.075f, 0.96f);
	const FLinearColor Panel(0.045f, 0.105f, 0.13f, 0.97f);
	const FLinearColor Yellow(1.0f, 0.78f, 0.16f, 1.0f);
	const FLinearColor Cyan(0.18f, 0.85f, 0.87f, 1.0f);
	const FLinearColor Muted(0.69f, 0.81f, 0.82f, 1.0f);
}

void UTDGameUI::ShowMainMenu() { CurrentPage = EPage::Main; BuildPage(); }
void UTDGameUI::ShowHelp() { CurrentPage = EPage::Help; BuildPage(); }
void UTDGameUI::ShowCommandMenu() { CurrentPage = EPage::Command; BuildPage(); }
void UTDGameUI::ShowTurretShop() { CurrentPage = EPage::TurretShop; BuildPage(); }
void UTDGameUI::ShowPauseMenu() { CurrentPage = EPage::Pause; BuildPage(); }
void UTDGameUI::ShowAirstrikeTargetMenu() { CurrentPage = EPage::Airstrike; BuildPage(); }
void UTDGameUI::ShowHUD() { CurrentPage = EPage::HUD; BuildPage(); RefreshDisplay(); }

UTextBlock* UTDGameUI::AddLabel(UVerticalBox* List, const FString& Label, int32 FontSize, const FLinearColor& Color, bool bBold)
{
	if (!List || !WidgetTree) return nullptr;
	UTextBlock* Text = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
	Text->SetText(FText::FromString(Label));
	Text->SetColorAndOpacity(FSlateColor(Color));
	Text->SetFont(FCoreStyle::GetDefaultFontStyle(bBold ? TEXT("Bold") : TEXT("Regular"), FontSize));
	Text->SetAutoWrapText(true);
	if (UVerticalBoxSlot* BoxSlot = List->AddChildToVerticalBox(Text)) BoxSlot->SetPadding(FMargin(0.0f, 4.0f));
	return Text;
}

UButton* UTDGameUI::AddButton(UVerticalBox* List, const FString& Label, const FLinearColor& Color)
{
	if (!List || !WidgetTree) return nullptr;
	UButton* Button = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass());
	Button->SetBackgroundColor(Color);
	Button->SetColorAndOpacity(FLinearColor(1, 1, 1, 1));
	UTextBlock* Text = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
	Text->SetText(FText::FromString(Label));
	Text->SetColorAndOpacity(FSlateColor(FLinearColor(0.02f, 0.045f, 0.055f, 1.0f)));
	Text->SetFont(FCoreStyle::GetDefaultFontStyle(TEXT("Bold"), 16));
	Text->SetJustification(ETextJustify::Center);
	Button->AddChild(Text);
	if (UVerticalBoxSlot* BoxSlot = List->AddChildToVerticalBox(Button)) BoxSlot->SetPadding(FMargin(0.0f, 5.0f));
	return Button;
}

void UTDGameUI::AddCommandButton(UVerticalBox* List, int32 Number, const FString& Name, const FString& Cost, const FString& Description)
{
	if (!List || !WidgetTree) return;
	UButton* Button = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass());
	Button->SetBackgroundColor(FLinearColor(0.10f, 0.19f, 0.21f, 1.0f));
	UVerticalBox* Content = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());
	UTextBlock* Heading = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
	const FString HeadingText = Number <= 7
		? FString::Printf(TEXT("%d   %s     %s credits"), Number, *Name, *Cost)
		: FString::Printf(TEXT("%s     %s credits"), *Name, *Cost);
	Heading->SetText(FText::FromString(HeadingText));
	Heading->SetColorAndOpacity(FSlateColor(TDUIStyle::Yellow));
	Heading->SetFont(FCoreStyle::GetDefaultFontStyle(TEXT("Bold"), 15));
	Content->AddChildToVerticalBox(Heading);
	UTextBlock* Detail = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
	Detail->SetText(FText::FromString(Description));
	Detail->SetColorAndOpacity(FSlateColor(TDUIStyle::Muted));
	Detail->SetFont(FCoreStyle::GetDefaultFontStyle(TEXT("Regular"), 12));
	Detail->SetAutoWrapText(true);
	Content->AddChildToVerticalBox(Detail);
	Button->AddChild(Content);
	if (UVerticalBoxSlot* BoxSlot = List->AddChildToVerticalBox(Button)) BoxSlot->SetPadding(FMargin(0.0f, 3.0f));
	switch (Number)
	{
	case 1: Button->OnClicked.AddDynamic(this, &UTDGameUI::Action1Clicked); break;
	case 2: Button->OnClicked.AddDynamic(this, &UTDGameUI::Action2Clicked); break;
	case 3: Button->OnClicked.AddDynamic(this, &UTDGameUI::Action3Clicked); break;
	case 4: Button->OnClicked.AddDynamic(this, &UTDGameUI::Action4Clicked); break;
	case 5: Button->OnClicked.AddDynamic(this, &UTDGameUI::Action5Clicked); break;
	case 6: Button->OnClicked.AddDynamic(this, &UTDGameUI::Action6Clicked); break;
	case 7: Button->OnClicked.AddDynamic(this, &UTDGameUI::Action7Clicked); break;
	case 8: Button->OnClicked.AddDynamic(this, &UTDGameUI::TurretMediumClicked); break;
	case 9: Button->OnClicked.AddDynamic(this, &UTDGameUI::TurretHeavyClicked); break;
	case 10: Button->OnClicked.AddDynamic(this, &UTDGameUI::TurretMineClicked); break;
	case 11: Button->OnClicked.AddDynamic(this, &UTDGameUI::TurretArtilleryClicked); break;
	default: break;
	}
}

void UTDGameUI::BuildPage()
{
	if (!WidgetTree) return;
	WaveText = nullptr; CreditsText = nullptr; ZombieText = nullptr; HealthText = nullptr; CameraText = nullptr; StatusText = nullptr; ControlHintText = nullptr; HealthBar = nullptr; TurretPlacementArrows.Reset();
	UCanvasPanel* Root = RootCanvas;
	if (!Root)
	{
		Root = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("TDRoot"));
		RootCanvas = Root;
		WidgetTree->RootWidget = Root;
	}
	else
	{
		Root->ClearChildren();
	}
	if (CurrentPage == EPage::HUD)
	{
		UBorder* TopPanel = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass());
		TopPanel->SetBrushColor(TDUIStyle::Ink);
		UCanvasPanelSlot* TopSlot = Root->AddChildToCanvas(TopPanel);
		TopSlot->SetAnchors(FAnchors(0.02f, 0.025f, 0.98f, 0.20f));
		TopSlot->SetOffsets(FMargin(0.0f));
		UVerticalBox* Top = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());
		TopPanel->AddChild(Top);
		UTextBlock* Title = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
		Title->SetText(FText::FromString(TEXT("LAST OUTPOST     /     MORROW DISTRICT")));
		Title->SetColorAndOpacity(FSlateColor(TDUIStyle::Yellow));
		Title->SetFont(FCoreStyle::GetDefaultFontStyle(TEXT("Bold"), 14));
		Top->AddChildToVerticalBox(Title);
		UHorizontalBox* Stats = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());
		Top->AddChildToVerticalBox(Stats);
		auto AddStat = [this, Stats](const FString& Initial, TObjectPtr<UTextBlock>& OutText)
		{
			OutText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
			OutText->SetText(FText::FromString(Initial));
			OutText->SetColorAndOpacity(FSlateColor(FLinearColor::White));
			OutText->SetFont(FCoreStyle::GetDefaultFontStyle(TEXT("Bold"), 18));
			if (UHorizontalBoxSlot* BoxSlot = Stats->AddChildToHorizontalBox(OutText)) BoxSlot->SetPadding(FMargin(0.0f, 5.0f, 28.0f, 2.0f));
		};
		AddStat(TEXT("WAVE 0/5"), WaveText);
		AddStat(TEXT("CREDITS 200"), CreditsText);
		AddStat(TEXT("ZOMBIES 0"), ZombieText);
		AddStat(TEXT("OUTPOST 100%"), HealthText);
		AddStat(TEXT("CAM: OUTPOST"), CameraText);
		HealthBar = WidgetTree->ConstructWidget<UProgressBar>(UProgressBar::StaticClass());
		HealthBar->SetFillColorAndOpacity(TDUIStyle::Cyan);
		HealthBar->SetPercent(1.0f);
		if (UVerticalBoxSlot* BoxSlot = Top->AddChildToVerticalBox(HealthBar)) BoxSlot->SetPadding(FMargin(0.0f, 3.0f, 40.0f, 0.0f));
		UBorder* Bottom = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass());
		Bottom->SetBrushColor(TDUIStyle::Ink);
		UCanvasPanelSlot* BottomSlot = Root->AddChildToCanvas(Bottom);
		BottomSlot->SetAnchors(FAnchors(0.02f, 0.84f, 0.98f, 0.97f));
		BottomSlot->SetOffsets(FMargin(0.0f));
		UVerticalBox* BottomContent = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());
		Bottom->AddChild(BottomContent);
		StatusText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
		StatusText->SetText(FText::FromString(TEXT("DEFEND THE OUTPOST  •  Build defenses from the command menu")));
		StatusText->SetColorAndOpacity(FSlateColor(TDUIStyle::Cyan));
		StatusText->SetFont(FCoreStyle::GetDefaultFontStyle(TEXT("Bold"), 14));
		BottomContent->AddChildToVerticalBox(StatusText);
		ControlHintText = AddLabel(BottomContent, TEXT("WASD / ARROWS  CHOOSE ROAD FROM OUTPOST     •     SPACE / HOME  RETURN TO OUTPOST     •     B  COMMANDS     •     ESC  PAUSE"), 12, TDUIStyle::Muted);
		for (int32 ArrowIndex = 0; ArrowIndex < 12; ++ArrowIndex)
		{
			UTextBlock* Arrow = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
			Arrow->SetText(FText::FromString(TEXT("|\nV")));
			Arrow->SetColorAndOpacity(FSlateColor(TDUIStyle::Yellow));
			Arrow->SetFont(FCoreStyle::GetDefaultFontStyle(TEXT("Bold"), 40));
			Arrow->SetJustification(ETextJustify::Center);
			Arrow->SetShadowColorAndOpacity(FLinearColor(0.02f, 0.03f, 0.04f, 0.95f));
			Arrow->SetShadowOffset(FVector2D(2.0f, 3.0f));
			UCanvasPanelSlot* ArrowSlot = Root->AddChildToCanvas(Arrow);
			ArrowSlot->SetAnchors(FAnchors(0.0f, 0.0f));
			ArrowSlot->SetAlignment(FVector2D(0.5f, 0.5f));
			ArrowSlot->SetSize(FVector2D(58.0f, 74.0f));
			Arrow->SetVisibility(ESlateVisibility::Collapsed);
			TurretPlacementArrows.Add(Arrow);
		}
		return;
	}

	UBorder* Dimmer = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass());
	Dimmer->SetBrushColor(FLinearColor(0.008f, 0.02f, 0.028f, CurrentPage == EPage::Main ? 0.78f : 0.72f));
	UCanvasPanelSlot* DimmerSlot = Root->AddChildToCanvas(Dimmer);
	DimmerSlot->SetAnchors(FAnchors(0, 0, 1, 1)); DimmerSlot->SetOffsets(FMargin(0));
	UBorder* Panel = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass());
	Panel->SetBrushColor(TDUIStyle::Panel);
	UCanvasPanelSlot* PanelSlot = Root->AddChildToCanvas(Panel);
	PanelSlot->SetAnchors(FAnchors(0.5f, 0.5f));
	PanelSlot->SetAlignment(FVector2D(0.5f, 0.5f));
	PanelSlot->SetPosition(FVector2D(0, 0));
	const bool bWidePanel = CurrentPage == EPage::Command;
	const bool bTurretPanel = CurrentPage == EPage::TurretShop;
	const bool bAirstrikePanel = CurrentPage == EPage::Airstrike;
	PanelSlot->SetSize(FVector2D(bWidePanel ? 680.0f : 560.0f, bWidePanel ? 680.0f : ((bAirstrikePanel || bTurretPanel) ? 500.0f : 560.0f)));
	Panel->SetPadding(FMargin(24.0f));
	UVerticalBox* Content = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());
	Panel->AddChild(Content);

	if (CurrentPage == EPage::Main)
	{
		AddLabel(Content, TEXT("LAST OUTPOST"), 36, TDUIStyle::Yellow, true);
		AddLabel(Content, TEXT("MORROW DISTRICT  /  DEFENSE COMMAND"), 13, TDUIStyle::Cyan, true);
		AddLabel(Content, TEXT("Five waves stand between the outpost and extraction. Build rooftop defenses, reinforce the walls, and hold until the helicopter can land."), 16, FLinearColor::White);
		UButton* Start = AddButton(Content, TEXT("START DEFENSE"), TDUIStyle::Yellow);
		if (Start) Start->OnClicked.AddDynamic(this, &UTDGameUI::StartClicked);
		UButton* How = AddButton(Content, TEXT("HOW TO PLAY"), TDUIStyle::Cyan);
		if (How) How->OnClicked.AddDynamic(this, &UTDGameUI::HelpClicked);
		AddLabel(Content, TEXT("TOWER DEFENSE  •  SOLO  •  LEVEL 01"), 12, TDUIStyle::Muted);
	}
	else if (CurrentPage == EPage::Help)
	{
		AddLabel(Content, TEXT("FIELD GUIDE"), 30, TDUIStyle::Yellow, true);
		AddLabel(Content, TEXT("THE MISSION"), 15, TDUIStyle::Cyan, true);
		AddLabel(Content, TEXT("Survive five zombie waves. Defenses fire automatically. When the final wave falls, reach the helicopter beyond the city exit."), 15, FLinearColor::White);
		AddLabel(Content, TEXT("COMMANDS"), 15, TDUIStyle::Cyan, true);
		AddLabel(Content, TEXT("B opens the build menu. Press 1 or choose Turret Types, select one of four towers, then click a yellow arrow above a free rooftop pad. Towers aim and reload automatically. Keys 2–7 still buy a bot, wire, repair, airstrike, turret upgrade and wall upgrade."), 15, FLinearColor::White);
		AddLabel(Content, TEXT("CAMERA VIEWS"), 15, TDUIStyle::Cyan, true);
		AddLabel(Content, TEXT("Use WASD or the arrow keys to fly along one of four straight camera lines. Every camera view points at the outpost. Press Space or Home to return above it before choosing a different road. The camera cannot stop between the five fixed views."), 15, FLinearColor::White);
		AddLabel(Content, TEXT("Earn 10 credits for each zombie defeated. Spend credits between waves to prepare."), 14, TDUIStyle::Muted);
		UButton* Back = AddButton(Content, TEXT("BACK"), TDUIStyle::Yellow);
		if (Back) Back->OnClicked.AddDynamic(this, &UTDGameUI::BackClicked);
	}
	else if (CurrentPage == EPage::Command)
	{
		AddLabel(Content, TEXT("OUTPOST COMMAND"), 26, TDUIStyle::Yellow, true);
		AddLabel(Content, TEXT("Choose a defense or upgrade. The game pauses while this panel is open."), 13, TDUIStyle::Muted);
		AddCommandButton(Content, 1, TEXT("CHOOSE TURRET TYPE"), TEXT("60–210"), TEXT("Pick a turret, then place it on a free rooftop pad. You only pay after choosing a spot."));
		AddCommandButton(Content, 2, TEXT("GUARD BOT"), TEXT("90"), TEXT("Mobile support that attacks zombies around the outpost."));
		AddCommandButton(Content, 3, TEXT("BARBED WIRE"), TEXT("35"), TEXT("Slows and damages zombies approaching the walls."));
		AddCommandButton(Content, 4, TEXT("REPAIR OUTPOST"), TEXT("40"), TEXT("Restore up to 300 integrity."));
		AddCommandButton(Content, 5, TEXT("EAGLE AIRSTRIKE"), TEXT("240"), TEXT("Choose one of the three zombie approaches. The strike hits the full lane."));
		AddCommandButton(Content, 6, TEXT("UPGRADE TURRET"), TEXT("50+"), TEXT("Improve the nearest turret. Each level costs more."));
		AddCommandButton(Content, 7, TEXT("REINFORCE WALLS"), TEXT("80"), TEXT("Reduce damage to the outpost. Up to three upgrades."));
		UButton* Close = AddButton(Content, TEXT("CLOSE  /  RESUME"), TDUIStyle::Cyan);
		if (Close) Close->OnClicked.AddDynamic(this, &UTDGameUI::ResumeClicked);
	}
	else if (CurrentPage == EPage::TurretShop)
	{
		AddLabel(Content, TEXT("ROOFTOP TURRETS"), 28, TDUIStyle::Yellow, true);
		AddLabel(Content, TEXT("Choose a tower. It will fire and reload by itself. Then click one of the yellow rooftop arrows."), 14, TDUIStyle::Muted);
		AddCommandButton(Content, 8, TEXT("MEDIUM AMMO TURRET"), TEXT("60"), TEXT("Fast automatic fire • medium damage • longest direct-fire range."));
		AddCommandButton(Content, 9, TEXT("HEAVY CANNON"), TEXT("125"), TEXT("Heavy rounds • high damage • fires about 3.7× slower than the medium turret."));
		AddCommandButton(Content, 10, TEXT("MINE LAUNCHER"), TEXT("160"), TEXT("Three mines per burst • mines arm after 1 second • strong splash damage • long range."));
		AddCommandButton(Content, 11, TEXT("HE ARTILLERY"), TEXT("210"), TEXT("Very slow high-arc shells • large blast • can target zombies anywhere on the map."));
		UButton* Close = AddButton(Content, TEXT("CANCEL  /  BACK TO GAME"), TDUIStyle::Cyan);
		if (Close) Close->OnClicked.AddDynamic(this, &UTDGameUI::ResumeClicked);
	}
	else if (CurrentPage == EPage::Pause)
	{
		AddLabel(Content, TEXT("PAUSED"), 32, TDUIStyle::Yellow, true);
		UButton* Resume = AddButton(Content, TEXT("RESUME DEFENSE"), TDUIStyle::Cyan);
		if (Resume) Resume->OnClicked.AddDynamic(this, &UTDGameUI::ResumeClicked);
		UButton* Command = AddButton(Content, TEXT("BUILD & UPGRADES"), TDUIStyle::Yellow);
		if (Command) Command->OnClicked.AddDynamic(this, &UTDGameUI::CommandClicked);
		UButton* Guide = AddButton(Content, TEXT("HOW TO PLAY"), TDUIStyle::Cyan);
		if (Guide) Guide->OnClicked.AddDynamic(this, &UTDGameUI::HelpClicked);
		UButton* Restart = AddButton(Content, TEXT("RESTART LEVEL"), FLinearColor(0.85f, 0.48f, 0.22f, 1));
		if (Restart) Restart->OnClicked.AddDynamic(this, &UTDGameUI::RestartClicked);
		UButton* Main = AddButton(Content, TEXT("RETURN TO MAIN MENU"), TDUIStyle::Yellow);
		if (Main) Main->OnClicked.AddDynamic(this, &UTDGameUI::MainMenuClicked);
		UButton* Exit = AddButton(Content, TEXT("EXIT GAME"), FLinearColor(0.72f, 0.30f, 0.25f, 1));
		if (Exit) Exit->OnClicked.AddDynamic(this, &UTDGameUI::ExitClicked);
	}
	else if (CurrentPage == EPage::Airstrike)
	{
		AddLabel(Content, TEXT("EAGLE AIRSTRIKE  /  240 CREDITS"), 24, TDUIStyle::Yellow, true);
		AddLabel(Content, TEXT("Choose one zombie approach. The strike covers the full lane and deals 250 damage to every zombie there."), 14, TDUIStyle::Muted);
		UButton* West = AddButton(Content, TEXT("1   WEST APPROACH"), TDUIStyle::Cyan);
		if (West) West->OnClicked.AddDynamic(this, &UTDGameUI::AirstrikeWestClicked);
		UButton* East = AddButton(Content, TEXT("2   EAST APPROACH"), TDUIStyle::Cyan);
		if (East) East->OnClicked.AddDynamic(this, &UTDGameUI::AirstrikeEastClicked);
		UButton* South = AddButton(Content, TEXT("3   SOUTH APPROACH"), TDUIStyle::Cyan);
		if (South) South->OnClicked.AddDynamic(this, &UTDGameUI::AirstrikeSouthClicked);
		UButton* Cancel = AddButton(Content, TEXT("CANCEL  /  ESC"), TDUIStyle::Yellow);
		if (Cancel) Cancel->OnClicked.AddDynamic(this, &UTDGameUI::AirstrikeCancelClicked);
	}
}

void UTDGameUI::RefreshDisplay()
{
	AextractionFPSGameMode* GM = GetWorld() ? GetWorld()->GetAuthGameMode<AextractionFPSGameMode>() : nullptr;
	if (!GM) return;
	if (WaveText) WaveText->SetText(FText::FromString(FString::Printf(TEXT("WAVE %d/5"), GM->GetCurrentWave())));
	if (CreditsText) CreditsText->SetText(FText::FromString(FString::Printf(TEXT("CREDITS %d"), GM->GetCredits())));
	if (ZombieText) ZombieText->SetText(FText::FromString(FString::Printf(TEXT("ZOMBIES %d"), GM->GetZombieCount())));
	if (HealthText) HealthText->SetText(FText::FromString(FString::Printf(TEXT("OUTPOST %d%%"), FMath::RoundToInt(GM->GetOutpostHealth() / 10.0f))));
	if (CameraText)
	{
		const AShooterPlayerController* PC = Cast<AShooterPlayerController>(GetOwningPlayer());
		CameraText->SetText(FText::FromString(FString::Printf(TEXT("CAM: %s"), PC ? *PC->GetCameraStopName() : TEXT("OUTPOST"))));
	}
	if (ControlHintText)
	{
		const AShooterPlayerController* PC = Cast<AShooterPlayerController>(GetOwningPlayer());
		const bool bPlacingTurret = PC && PC->IsTurretPlacementPending();
		FString Hint = bPlacingTurret
			? TEXT("WASD / ARROWS  SHOW ROOFTOP PADS     •     CLICK A YELLOW ARROW TO PLACE     •     RIGHT CLICK / ESC  CANCEL")
			: TEXT("WASD / ARROWS  CHOOSE ROAD FROM OUTPOST     •     SPACE / HOME  RETURN TO OUTPOST     •     B  COMMANDS     •     ESC  PAUSE");
		if (bPlacingTurret)
		{
			TArray<FVector2D> DebugPositions;
			PC->GetTurretPlacementScreenPositions(DebugPositions);
			int32 VisibleSlots = 0;
			for (const FVector2D& Position : DebugPositions)
			{
				if (Position.X > -1000.0f) ++VisibleSlots;
			}
			Hint += FString::Printf(TEXT("     •     ROOFTOP PADS IN VIEW: %d/%d"), VisibleSlots, DebugPositions.Num());
		}
		ControlHintText->SetText(FText::FromString(Hint));
		ControlHintText->SetColorAndOpacity(FSlateColor(bPlacingTurret ? TDUIStyle::Yellow : TDUIStyle::Muted));
	}
	if (const AShooterPlayerController* PC = Cast<AShooterPlayerController>(GetOwningPlayer()))
	{
		TArray<FVector2D> MarkerPositions;
		PC->GetTurretPlacementScreenPositions(MarkerPositions);
		for (int32 Index = 0; Index < TurretPlacementArrows.Num(); ++Index)
		{
			UTextBlock* Arrow = TurretPlacementArrows[Index];
			if (!Arrow) continue;
			const float ViewportScale = FMath::Max(UWidgetLayoutLibrary::GetViewportScale(this), 0.01f);
			const FVector2D Position = MarkerPositions.IsValidIndex(Index) ? MarkerPositions[Index] : FVector2D(-10000.0f, -10000.0f);
			if (PC->IsTurretPlacementPending() && Position.X > -1000.0f)
			{
				if (UCanvasPanelSlot* ArrowSlot = Cast<UCanvasPanelSlot>(Arrow->Slot))
				{
					ArrowSlot->SetPosition(Position / ViewportScale);
				}
				Arrow->SetVisibility(ESlateVisibility::HitTestInvisible);
			}
			else
			{
				Arrow->SetVisibility(ESlateVisibility::Collapsed);
			}
		}
	}
	if (HealthBar) HealthBar->SetPercent(FMath::Clamp(GM->GetOutpostHealth() / 1000.0f, 0.0f, 1.0f));
	if (StatusText) StatusText->SetText(FText::FromString(GM->GetLastDefenseStatus()));
}

void UTDGameUI::DoAction(uint8 Number)
{
	if (AShooterPlayerController* PC = Cast<AShooterPlayerController>(GetOwningPlayer())) PC->ExecuteDefenseAction(Number);
}
void UTDGameUI::StartClicked() { if (AShooterPlayerController* PC = Cast<AShooterPlayerController>(GetOwningPlayer())) PC->StartDefense(); }
void UTDGameUI::HelpClicked() { if (AShooterPlayerController* PC = Cast<AShooterPlayerController>(GetOwningPlayer())) PC->ShowHowTo(); }
void UTDGameUI::BackClicked() { if (AShooterPlayerController* PC = Cast<AShooterPlayerController>(GetOwningPlayer())) PC->ReturnToCurrentMenu(); }
void UTDGameUI::ResumeClicked() { if (AShooterPlayerController* PC = Cast<AShooterPlayerController>(GetOwningPlayer())) PC->CloseOverlayAndResume(); }
void UTDGameUI::CommandClicked() { if (AShooterPlayerController* PC = Cast<AShooterPlayerController>(GetOwningPlayer())) PC->ToggleCommandMenu(); }
void UTDGameUI::RestartClicked() { if (AShooterPlayerController* PC = Cast<AShooterPlayerController>(GetOwningPlayer())) PC->RestartLevel(); }
void UTDGameUI::MainMenuClicked() { if (AShooterPlayerController* PC = Cast<AShooterPlayerController>(GetOwningPlayer())) PC->ShowMainMenu(); }
void UTDGameUI::ExitClicked() { if (APlayerController* PC = GetOwningPlayer()) UKismetSystemLibrary::QuitGame(this, PC, EQuitPreference::Quit, false); }
void UTDGameUI::Action1Clicked() { DoAction(1); }
void UTDGameUI::Action2Clicked() { DoAction(2); }
void UTDGameUI::Action3Clicked() { DoAction(3); }
void UTDGameUI::Action4Clicked() { DoAction(4); }
void UTDGameUI::Action5Clicked() { DoAction(5); }
void UTDGameUI::Action6Clicked() { DoAction(6); }
void UTDGameUI::Action7Clicked() { DoAction(7); }
void UTDGameUI::TurretMediumClicked() { if (AShooterPlayerController* PC = Cast<AShooterPlayerController>(GetOwningPlayer())) PC->BeginTurretPlacement(ETDDefenseType::Turret); }
void UTDGameUI::TurretHeavyClicked() { if (AShooterPlayerController* PC = Cast<AShooterPlayerController>(GetOwningPlayer())) PC->BeginTurretPlacement(ETDDefenseType::HeavyTurret); }
void UTDGameUI::TurretMineClicked() { if (AShooterPlayerController* PC = Cast<AShooterPlayerController>(GetOwningPlayer())) PC->BeginTurretPlacement(ETDDefenseType::MineTurret); }
void UTDGameUI::TurretArtilleryClicked() { if (AShooterPlayerController* PC = Cast<AShooterPlayerController>(GetOwningPlayer())) PC->BeginTurretPlacement(ETDDefenseType::ArtilleryTurret); }
void UTDGameUI::AirstrikeWestClicked() { if (AShooterPlayerController* PC = Cast<AShooterPlayerController>(GetOwningPlayer())) PC->ConfirmAirstrikeTarget(0); }
void UTDGameUI::AirstrikeEastClicked() { if (AShooterPlayerController* PC = Cast<AShooterPlayerController>(GetOwningPlayer())) PC->ConfirmAirstrikeTarget(1); }
void UTDGameUI::AirstrikeSouthClicked() { if (AShooterPlayerController* PC = Cast<AShooterPlayerController>(GetOwningPlayer())) PC->ConfirmAirstrikeTarget(2); }
void UTDGameUI::AirstrikeCancelClicked() { if (AShooterPlayerController* PC = Cast<AShooterPlayerController>(GetOwningPlayer())) PC->CancelAirstrikeTarget(); }
