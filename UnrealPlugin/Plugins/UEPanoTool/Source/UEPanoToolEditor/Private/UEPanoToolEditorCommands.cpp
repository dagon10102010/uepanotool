// Copyright Epic Games, Inc. All Rights Reserved.

#include "UEPanoToolEditorCommands.h"

#define LOCTEXT_NAMESPACE "FUEPanoToolEditorModule"

void FUEPanoToolEditorCommands::RegisterCommands()
{
	UI_COMMAND(OpenPluginWindow, "UEPanoToolEditor", "Bring up UEPanoToolEditor window", EUserInterfaceActionType::Button, FInputChord());
}

#undef LOCTEXT_NAMESPACE
