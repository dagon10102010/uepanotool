// Copyright Epic Games, Inc. All Rights Reserved.

#include "UEPanoToolEditor.h"
#include "UEPanoToolEditorStyle.h"
#include "UEPanoToolEditorCommands.h"
#include "LevelEditor.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Text/STextBlock.h"
#include "ToolMenus.h"
#include "PanoToolWidget.h"

static const FName UEPanoToolEditorTabName("UEPanoToolEditor");

#define LOCTEXT_NAMESPACE "FUEPanoToolEditorModule"

void FUEPanoToolEditorModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	
	FUEPanoToolEditorStyle::Initialize();
	FUEPanoToolEditorStyle::ReloadTextures();

	FUEPanoToolEditorCommands::Register();
	
	PluginCommands = MakeShareable(new FUICommandList);

	PluginCommands->MapAction(
		FUEPanoToolEditorCommands::Get().OpenPluginWindow,
		FExecuteAction::CreateRaw(this, &FUEPanoToolEditorModule::PluginButtonClicked),
		FCanExecuteAction());

	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FUEPanoToolEditorModule::RegisterMenus));
	
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(UEPanoToolEditorTabName, FOnSpawnTab::CreateRaw(this, &FUEPanoToolEditorModule::OnSpawnPluginTab))
		.SetDisplayName(LOCTEXT("FUEPanoToolEditorTabTitle", "UEPanoToolEditor"))
		.SetMenuType(ETabSpawnerMenuType::Hidden);
}

void FUEPanoToolEditorModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.

	UToolMenus::UnRegisterStartupCallback(this);

	UToolMenus::UnregisterOwner(this);

	FUEPanoToolEditorStyle::Shutdown();

	FUEPanoToolEditorCommands::Unregister();

	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(UEPanoToolEditorTabName);
}

TSharedRef<SDockTab> FUEPanoToolEditorModule::OnSpawnPluginTab(const FSpawnTabArgs& SpawnTabArgs)
{
	

	return SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		[
			// Put your tab content here!
			SNew(SHorizontalBox)
			+SHorizontalBox::Slot()
			.Padding(5)
			[
				SNew(SPanoToolWidget)
			]
		];
}

void FUEPanoToolEditorModule::PluginButtonClicked()
{
	FGlobalTabmanager::Get()->TryInvokeTab(UEPanoToolEditorTabName);
}

void FUEPanoToolEditorModule::RegisterMenus()
{
	// Owner will be used for cleanup in call to UToolMenus::UnregisterOwner
	FToolMenuOwnerScoped OwnerScoped(this);

	{
		UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Window");
		{
			FToolMenuSection& Section = Menu->FindOrAddSection("WindowLayout");
			Section.AddMenuEntryWithCommandList(FUEPanoToolEditorCommands::Get().OpenPluginWindow, PluginCommands);
		}
	}

	{
		UToolMenu* ToolbarMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar");
		{
			FToolMenuSection& Section = ToolbarMenu->FindOrAddSection("Settings");
			{
				FToolMenuEntry& Entry = Section.AddEntry(FToolMenuEntry::InitToolBarButton(FUEPanoToolEditorCommands::Get().OpenPluginWindow));
				Entry.SetCommandList(PluginCommands);
			}
		}
	}
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FUEPanoToolEditorModule, UEPanoToolEditor)