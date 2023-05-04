#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "PanoToolData.h"

class SPanoToolWidget : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(SPanoToolWidget)
	{
	}
	SLATE_END_ARGS()

    void Construct(const FArguments& InArgs);

	// /** Gets the widget contents of the app */
	// virtual TSharedRef<SWidget> GetContent();

	// virtual ~SPanoToolWidget();

	// /** SWidget interface */
	// virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;

    // TSharedPtr<class SButton> ButtonCreateSkyLight;
private:

	TArray<TSharedPtr<FString>> CaptureStyleStrings;
	TArray<TSharedPtr<FString>> OutputStrings;
	FPanoCaptureStyleData captureStyleData = FPanoCaptureStyleData();
	FPanoCaptureConfig captureConfig = FPanoCaptureConfig();
	int outputWidth = 4096;
	int step = 2;

	FReply TestClick();
	
	FReply CreateRenderScript();
	FReply ResetMovieSequence();
    FReply CreateStaticRenderScript();
	void CalcSize();
	void OnChangeCaptureStyle( TSharedPtr<FString> NewSelection, ESelectInfo::Type SelectInfo );
	void OnChangeOutput( TSharedPtr<FString> NewSelection, ESelectInfo::Type SelectInfo );
	void OnChangeWidth(int width);
	void OnChangeStep(int s);
	void GenSequence();
	void ResetSequence();
	void SaveVideoScriptFile();
	void SaveScriptFile(FPanoPointList* list);
	FString renderArg = L"-game -PIEVIACONSOLE -Multiprocess  GameUserSettingsINI=\"PIEGameUserSettings0\" -MultiprocessSaveConfig -forcepassthrough -messaging -SessionName=\"Play in Standalone Game\" -windowed -WinX=320 -WinY=194 -ResX=1280 -ResY=720";
	FString RenderCmd(int index);
};

