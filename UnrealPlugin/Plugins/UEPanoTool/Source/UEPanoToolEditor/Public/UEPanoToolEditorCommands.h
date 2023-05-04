// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"
#include "UEPanoToolEditorStyle.h"

class FUEPanoToolEditorCommands : public TCommands<FUEPanoToolEditorCommands>
{
public:

	FUEPanoToolEditorCommands()
		: TCommands<FUEPanoToolEditorCommands>(TEXT("UEPanoToolEditor"), NSLOCTEXT("Contexts", "UEPanoToolEditor", "UEPanoToolEditor Plugin"), NAME_None, FUEPanoToolEditorStyle::GetStyleSetName())
	{
	}

	// TCommands<> interface
	virtual void RegisterCommands() override;

public:
	TSharedPtr< FUICommandInfo > OpenPluginWindow;
};