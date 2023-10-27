#include "PanoToolWidget.h"
#include "LevelEditor.h"
#include "LevelEditorViewport.h"
#include "Widgets/Input/SSpinBox.h"
#include "Widgets/Input/STextComboBox.h"
#include "JsonObjectConverter.h"
#include "Misc/FileHelper.h"
#include "Kismet/GameplayStatics.h"
#include "PanoHotspot.h"
#include "PanoCapture.h"
#include "LevelSequence.h"
#include "MovieScene.h"
#include "EditorAssetLibrary.h"
#include "Tracks/MovieSceneCameraCutTrack.h"
#include "Tracks/MovieScene3DTransformTrack.h"
#include "Sections/MovieSceneCameraCutSection.h"
#include "Channels/MovieSceneChannelProxy.h"
#include "EditorDialogLibrary.h"
#define LOCTEXT_NAMESPACE "PanoTool"
void SPanoToolWidget::Construct(const FArguments& InArgs){
    
    FString str;
    
	FString fPath = FPaths::Combine(FPaths::ProjectPluginsDir(), TEXT("UEPanoTool/Content/capturestyle.json"));
    if (FFileHelper::LoadFileToString(str, *fPath))
        FJsonObjectConverter::JsonObjectStringToUStruct(str,&captureStyleData);
    captureConfig.Load();
    
    captureConfig.ued = FPaths::ConvertRelativePathToFull(FPaths::EngineDir() + L"Binaries/Win64/UnrealEditor.exe").Replace(L"/",L"\\");
	captureConfig.project = FPaths::ConvertRelativePathToFull(FPaths::GetProjectFilePath());
    int w = captureConfig.data.outwidth;
    int CaptureStyleSelectedIndex = 0;
    for (size_t i = 0; i < captureStyleData.data.Num(); i++)
    {
        if(captureStyleData.data[i].name==captureConfig.data.name){
            CaptureStyleSelectedIndex = i;
        }
    }
    captureConfig.data = captureStyleData.data[CaptureStyleSelectedIndex];
    captureConfig.data.outwidth = w;
    CaptureStyleStrings.Empty();
    
    
    CaptureStyleStrings.Add(MakeShareable(new FString(LOCTEXT("PanoTool", "mono6").ToString())));
    CaptureStyleStrings.Add(MakeShareable(new FString(LOCTEXT("PanoTool", "mono26").ToString())));
    CaptureStyleStrings.Add(MakeShareable(new FString(LOCTEXT("PanoTool", "mono36").ToString())));
    CaptureStyleStrings.Add(MakeShareable(new FString(LOCTEXT("PanoTool", "stereo240").ToString())));
    CaptureStyleStrings.Add(MakeShareable(new FString(LOCTEXT("PanoTool", "stereo2160").ToString())));
    
    
    
    OutputStrings.Empty();
    OutputStrings.Add(MakeShareable(new FString(LOCTEXT("PanoTool", "EquirectAngular").ToString())));
    OutputStrings.Add(MakeShareable(new FString(LOCTEXT("PanoTool", "Google EAC").ToString())));
    OutputStrings.Add(MakeShareable(new FString(LOCTEXT("PanoTool", "Cube").ToString())));
    OutputStrings.Add(MakeShareable(new FString(LOCTEXT("PanoTool", "VR180").ToString())));
    OutputStrings.Add(MakeShareable(new FString(LOCTEXT("PanoTool", "Fisheye180").ToString())));

    this->ChildSlot
    [
        SNew(SVerticalBox)
        +SVerticalBox::Slot()
        .AutoHeight()
        [
            SNew(SHorizontalBox)
            +SHorizontalBox::Slot()
            .FillWidth(1)
            [
                SNew(STextBlock)
                .Text( LOCTEXT("PanoTool", "Output Width"))
            ]
            +SHorizontalBox::Slot()
            .FillWidth(2)
            [
                SNew(SSpinBox<int>)
                .MinValue(1024)
                .MaxValue(16384)
                .Value(captureConfig.data.outwidth)
                .OnValueChanged(this,&SPanoToolWidget::OnChangeWidth)
            ]
            
        ]
        +SVerticalBox::Slot()
        .AutoHeight()
        [
            SNew(SHorizontalBox)
            +SHorizontalBox::Slot()
            .FillWidth(1)
            [
                SNew(STextBlock)
                .Text( LOCTEXT("PanoTool", "Capture Style"))
            ]
            +SHorizontalBox::Slot()
            .FillWidth(2)
            [
                SNew(STextComboBox)
                .OptionsSource(&CaptureStyleStrings)
                .InitiallySelectedItem(CaptureStyleStrings[CaptureStyleSelectedIndex])
                .OnSelectionChanged(this, &SPanoToolWidget::OnChangeCaptureStyle)
            ]
        ]
        +SVerticalBox::Slot()
        .AutoHeight()
        [
            SNew(SHorizontalBox)
            +SHorizontalBox::Slot()
            .FillWidth(1)
            [
                SNew(STextBlock)
                .Text( LOCTEXT("PanoTool", "Output"))
            ]
            +SHorizontalBox::Slot()
            .FillWidth(2)
            [
                SNew(STextComboBox)
                .OptionsSource(&OutputStrings)
                .InitiallySelectedItem(OutputStrings[captureConfig.projector])
                .OnSelectionChanged(this, &SPanoToolWidget::OnChangeOutput)
            ]
        ]
        +SVerticalBox::Slot()
        .AutoHeight()
        [
            SNew(SHorizontalBox)
            +SHorizontalBox::Slot()
            .FillWidth(1)
            [
                SNew(STextBlock)
                .Text( LOCTEXT("PanoTool", "Output directory"))
            ]
            +SHorizontalBox::Slot()
            .FillWidth(2)
            [
                SNew(SEditableTextBox )
                .Text(FText::FromString(captureConfig.outputdir))
                .OnTextCommitted(this,&SPanoToolWidget::OnChangeDir)
            ]
        ]
        +SVerticalBox::Slot()
        .AutoHeight()
        [
            SNew(SHorizontalBox)
            +SHorizontalBox::Slot()
            .FillWidth(1)
            [
                SNew(STextBlock)
                .Text( LOCTEXT("PanoTool", "Step"))
            ]
            +SHorizontalBox::Slot()
            .FillWidth(2)
            [
                SNew(SSpinBox<int>)
                .MinValue(2)
                .MaxValue(10)
                .Value(captureConfig.step)
                .OnValueChanged(this,&SPanoToolWidget::OnChangeStep)
            ]
            
        ]
        +SVerticalBox::Slot()
        .AutoHeight()
        [
            SNew(SHorizontalBox)
            +SHorizontalBox::Slot()
            .FillWidth(3)
            [
                SNew(STextBlock)
                .Text( LOCTEXT("PanoTool", "Remove images when finish"))
            ]
            +SHorizontalBox::Slot()
            .FillWidth(1)
            [
                SNew(SCheckBox)
                .IsChecked(captureConfig.clearImg)
                .OnCheckStateChanged(this,&SPanoToolWidget::OnChangeDeleteImg)
            ]
            +SHorizontalBox::Slot()
            .FillWidth(2)
            [
                SNew(STextBlock)
                .Text( LOCTEXT("PanoTool", "Manual stitch"))
            ]
            +SHorizontalBox::Slot()
            .FillWidth(1)
            [
                SNew(SCheckBox)
                .IsChecked(!captureConfig.autoStitch)
                .OnCheckStateChanged(this,&SPanoToolWidget::OnChangeStitch)
            ]
            
        ]
        +SVerticalBox::Slot()
        .AutoHeight()
        [
            SNew(SHorizontalBox)
            +SHorizontalBox::Slot()
            .FillWidth(2)
            [
                SNew(STextBlock)
                .Text( LOCTEXT("PanoTool", "Save tiff"))
            ]
            +SHorizontalBox::Slot()
            .FillWidth(1)
            [
                SNew(SCheckBox)
                .IsChecked(captureConfig.tiff)
                .OnCheckStateChanged(this,&SPanoToolWidget::OnChangeTif)
            ]
            +SHorizontalBox::Slot()
            .FillWidth(2)
            [
                SNew(STextBlock)
                .Text( LOCTEXT("PanoTool", "Save jpg"))
            ]            
            +SHorizontalBox::Slot()
            .FillWidth(1)
            [
                SNew(SCheckBox)
                .IsChecked(captureConfig.jpg)
                .OnCheckStateChanged(this,&SPanoToolWidget::OnChangeJpg)
            ]
            +SHorizontalBox::Slot()
            .FillWidth(2)
            [
                SNew(STextBlock)
                .Text( LOCTEXT("PanoTool", "Save png"))
            ]
            +SHorizontalBox::Slot()
            .FillWidth(1)
            [
                SNew(SCheckBox)
                .IsChecked(captureConfig.png)
                .OnCheckStateChanged(this,&SPanoToolWidget::OnChangePng)
            ]
            
        ]
        +SVerticalBox::Slot()
        .AutoHeight()
        [
            SNew(SHorizontalBox)
            +SHorizontalBox::Slot()
            .FillWidth(1)
            [
                SNew(SButton)
                .HAlign(EHorizontalAlignment::HAlign_Center)
                .Text( LOCTEXT("PanoTool", "Reset Movie Sequence"))
                .OnClicked(this,&SPanoToolWidget::ResetMovieSequence)
            ]
        ]
        +SVerticalBox::Slot()
        .AutoHeight()
        [
            SNew(SHorizontalBox)
            +SHorizontalBox::Slot()
            .FillWidth(1)
            [
                SNew(SButton)
                .HAlign(EHorizontalAlignment::HAlign_Center)
                .Text( LOCTEXT("PanoTool", "Create Video MRQ"))
                .OnClicked(this,&SPanoToolWidget::GenMRQ)
            ]
        ]
        +SVerticalBox::Slot()
        .AutoHeight()
        [
            SNew(SHorizontalBox)
            +SHorizontalBox::Slot()
            .FillWidth(1)
            [
                SNew(SButton)
                .HAlign(EHorizontalAlignment::HAlign_Center)
                .Text( LOCTEXT("PanoTool", "Create Video Script"))
                .OnClicked(this,&SPanoToolWidget::CreateRenderScript)
            ]
            +SHorizontalBox::Slot()
            .FillWidth(1)
            [
                SNew(SButton)
                .HAlign(EHorizontalAlignment::HAlign_Center)
                .Text( LOCTEXT("PanoTool", "Create Photo Script"))
                .OnClicked(this,&SPanoToolWidget::CreateStaticRenderScript)
            ]
        ]
    ];
}
void SPanoToolWidget::OnChangeWidth(int width){
    captureConfig.data.outwidth = width;
}
void SPanoToolWidget::OnChangeStep(int s){
    captureConfig.step = s;
}
void SPanoToolWidget::OnChangeDeleteImg(ECheckBoxState state){
    captureConfig.clearImg = state==ECheckBoxState::Checked;
}
void SPanoToolWidget::OnChangeTif( ECheckBoxState state){
    captureConfig.tiff = state==ECheckBoxState::Checked;
}
void SPanoToolWidget::OnChangeJpg( ECheckBoxState state){
    captureConfig.jpg = state==ECheckBoxState::Checked;
}
void SPanoToolWidget::OnChangePng( ECheckBoxState state){
    captureConfig.png = state==ECheckBoxState::Checked;
}
void SPanoToolWidget::OnChangeStitch( ECheckBoxState state){
    captureConfig.autoStitch = state!=ECheckBoxState::Checked;
}

void SPanoToolWidget::OnChangeDir( const FText & text, ETextCommit::Type type){
    captureConfig.outputdir = text.ToString();
}
void SPanoToolWidget::OnChangeCaptureStyle( TSharedPtr<FString> NewSelection, ESelectInfo::Type SelectInfo ){
    int32 Index;
	bool bFound = CaptureStyleStrings.Find(NewSelection, Index);
	check(bFound);
    captureConfig.data = captureStyleData.data[Index];
}
void SPanoToolWidget::OnChangeOutput( TSharedPtr<FString> NewSelection, ESelectInfo::Type SelectInfo ){
    int32 Index;
	bool bFound = OutputStrings.Find(NewSelection, Index);
	check(bFound);
    captureConfig.projector = Index;
}
void SPanoToolWidget::CalcSize(){
    captureConfig.data.outheight = captureConfig.data.outwidth/2;
    float outrad = 3.1416f;
    if(captureConfig.projector==3 || captureConfig.projector==4){
        captureConfig.data.outheight = captureConfig.data.outwidth;
        outrad *= 0.5;
    }
    int w = captureConfig.data.outwidth*FMath::Tan(FMath::DegreesToRadians(captureConfig.data.fov)*0.5f)/outrad;
    captureConfig.data.width = w-w%captureConfig.data.basewidth+captureConfig.data.basewidth;
    captureConfig.data.height = captureConfig.data.baseheight*captureConfig.data.width/captureConfig.data.basewidth;
    UWorld* World = GEditor->GetEditorWorldContext().World();
    FString lname = World->GetCurrentLevel()->GetPathName();
    FString fname = "";
    FString sname = "";
    lname.Split(L".",&fname,&sname);
    captureConfig.map = fname;
}
FReply SPanoToolWidget::GenMRQ(){
    return FReply::Handled();
}
FReply SPanoToolWidget::CreateRenderScript(){
    CalcSize();
    captureConfig.video = true;
    captureConfig.Save();
    SaveVideoScriptFile();
	return FReply::Handled();
}
FReply SPanoToolWidget::ResetMovieSequence(){
    ResetSequence();
	return FReply::Handled();
}
FReply SPanoToolWidget::CreateStaticRenderScript(){
    CalcSize();
    captureConfig.video = false;
    GenSequence();
    captureConfig.Save();
	return FReply::Handled();
}
FReply SPanoToolWidget::TestClick(){
    
	return FReply::Handled();
}
FString SPanoToolWidget::RenderCmd(int index){
    return FString().Printf(L"\"%s\" \"%s\" %s %s -renderpano -renderindex %d\n",*captureConfig.ued,*captureConfig.project,*captureConfig.map,*renderArg,index);
}
void SPanoToolWidget::SaveVideoScriptFile(){
    APanoCapture* panocapture = CheckMap();
    if(!panocapture)
        return;
    FString cmd = FString().Printf(L"copy .\\Plugins\\UEPanoTool\\Tool\\uestitch.exe \"%s\" /y\n",*captureConfig.outputdir);
    FString stitchcmd = FString("pushd \"%~dp0\"\nmkdir MovieRenders_output\n");
    FString script = FString().Printf(L"width %d\nheight %d\nprojector %d\nalpha 1\n",captureConfig.data.outwidth,captureConfig.data.outheight,captureConfig.projector);
    for (int i = 0; i < captureConfig.data.total; i++){
        FVector r = captureConfig.data.GetRotate(i);
        if(!captureConfig.IsFrontFace(i)){
            continue;
        }
        script += FString().Printf(L"plane s%d %f %f 0 %f %d %d\n",i,r.Z,r.Y,captureConfig.data.fov,captureConfig.data.width,captureConfig.data.height);
    }
    UMovieSceneSequence* captureSeq = Cast<ULevelSequence>(UEditorAssetLibrary::LoadAsset(L"/UEPanoTool/Animation/renderseqvideo"));
    UMovieScene* ms = captureSeq->GetMovieScene();
    FFrameRate DisplayRate = ms->GetDisplayRate();
    captureConfig.startframe = ConvertFrameTime(UE::MovieScene::DiscreteInclusiveLower(ms->GetPlaybackRange()), ms->GetTickResolution(), DisplayRate).FloorToFrame().Value;
    captureConfig.endframe = ConvertFrameTime(UE::MovieScene::DiscreteExclusiveUpper(ms->GetPlaybackRange()), ms->GetTickResolution(), DisplayRate).FloorToFrame().Value;
    TArray<FString> ls = panocapture->GetLayerObj();
    script += FString().Printf(L"loop %d %d\n",captureConfig.startframe,captureConfig.endframe);

    for (FString l : ls)
    {
        
        for (int i = 0; i < captureConfig.data.total; i++){
            if(!captureConfig.IsFrontFace(i)){
                continue;
            }
            script += FString().Printf(L"input s%d MovieRenders/%s.%d%s.%%04d.png\n",i,*captureConfig.data.GetEyeName(i),i,*l);
        }
        script += BuildOutputString(0,l);
        script += FString().Printf(L"clear\n");
        if(captureConfig.data.stereo){
            for (int i = 0; i < captureConfig.data.total; i++){
                if(!captureConfig.IsFrontFace(i)){
                    continue;
                }
                script += FString().Printf(L"input s%d MovieRenders/%s.%d%s.%%04d.png\n",i,*captureConfig.data.GetEyeName(i+captureConfig.data.total),i,*l);
            }
            // script += FString().Printf(L"output MovieRenders_output/right.%%04d.tif\n");
            // if(captureConfig.jpg)
            //     script += FString().Printf(L"output MovieRenders_output/right.%%04d.jpg\n");
            script += BuildOutputString(1,l);
            script += FString().Printf(L"clear\n");
        }
    }
    
    script += FString().Printf(L"endloop\n");
    for (int i = 0; i < captureConfig.data.total; i++){
        if(!captureConfig.IsFrontFace(i)){
            continue;
        }
        cmd += RenderCmd(i);
    }
    if(captureConfig.data.stereo){
        for (int i = 0; i < captureConfig.data.total; i++){
            if(!captureConfig.IsFrontFace(i)){
                continue;
            }
            cmd += RenderCmd(i+captureConfig.data.total);
        }
    }

    FString fPath = FPaths::Combine(captureConfig.outputdir,L"script.txt");
    FFileHelper::SaveStringToFile(script, *fPath);

    stitchcmd += FString().Printf(L"uestitch -script script.txt\n");
    if(captureConfig.data.stereo){
        script = FString().Printf(L"width %d\nheight %d\n\n",captureConfig.data.outwidth,captureConfig.data.outheight*2);
        script += FString().Printf(L"loop %d %d\n",captureConfig.startframe,captureConfig.endframe);
        script += FString().Printf(L"background MovieRenders_output/left.%%04d.tif\n");
        script += FString().Printf(L"background MovieRenders_output/right.%%04d.tif %d\n",captureConfig.data.outheight);
        if(captureConfig.jpg)
            script += FString().Printf(L"output MovieRenders_output/%%04d.jpg\n");
        script += L"clear\n";
        script += L"endloop\n";
        fPath = FPaths::Combine(captureConfig.outputdir,L"scriptjoin.txt");
        FFileHelper::SaveStringToFile(script, *fPath);
        stitchcmd += FString().Printf(L"uestitch -script scriptjoin.txt\n");
    }
    if(captureConfig.clearImg)
        stitchcmd += L"rmdir MovieRenders /S /Q\n";

    
    if(captureConfig.autoStitch){
        cmd += FString().Printf(L"pushd \"%s\"\n",*captureConfig.outputdir);
        cmd += L"start stitch.cmd\n";
        cmd += L"popd\n";
    }
    fPath = FPaths::Combine(FPaths::ProjectDir(),L"render.cmd");
    FFileHelper::SaveStringToFile(cmd, *fPath);

    stitchcmd += L"popd\n";
    fPath = FPaths::Combine(captureConfig.outputdir,L"stitch.cmd");
    FFileHelper::SaveStringToFile(stitchcmd, *fPath);
}
FString SPanoToolWidget::BuildOutputString(int eye,FString layer){
    FString script = FString();
    if(captureConfig.tiff){
        script += FString().Printf(L"output MovieRenders_output/%s.%s.%%04d.tif\n",*captureConfig.data.GetEyeName(0),*layer);
    }
    if(captureConfig.png){
        script += FString().Printf(L"output MovieRenders_output/%s.%s.%%04d.png\n",*captureConfig.data.GetEyeName(0),*layer);
    }
    if(captureConfig.jpg){
        script += FString().Printf(L"output MovieRenders_output/%s.%s.%%04d.jpg\n",*captureConfig.data.GetEyeName(0),*layer);
    }
    if(!captureConfig.tiff && !captureConfig.png && !captureConfig.jpg){
        script += FString().Printf(L"output MovieRenders_output/%s.%s.%%04d.jpg\n",*captureConfig.data.GetEyeName(0),*layer);
    }
    return script;
}
void SPanoToolWidget::SaveScriptFile(FPanoPointList* list){
    APanoCapture* panocapture = CheckMap();
    if(!panocapture)
        return;
    FString cmd = FString().Printf(L"copy .\\Plugins\\UEPanoTool\\Tool\\uestitch.exe \"%s\" /y\n",*captureConfig.outputdir);
    FString stitchcmd = FString("pushd \"%~dp0\"\nmkdir MovieRenders_output\n");
    cmd += RenderCmd(0);
    
    stitchcmd += FString().Printf(L"uestitch -script script.txt\n");
    

    FString script = FString().Printf(L"width %d\nheight %d\nprojector %d\n",captureConfig.data.outwidth,captureConfig.data.outheight,captureConfig.projector);
    for (int i = 0; i < captureConfig.data.total; i++){
        if(!captureConfig.IsFrontFace(i)){
            continue;
        }
        FVector r = captureConfig.data.GetRotate(i);
        script += FString().Printf(L"plane s%d %f %f 0 %f %d %d\n",i,r.Z,r.Y,captureConfig.data.fov,captureConfig.data.width,captureConfig.data.height);
    }
    int count = 0;
    for (FPanoPoint p : list->data){
        for (int i = 0; i < captureConfig.data.total; i++)
        {
            if(captureConfig.IsFrontFace(i)){
                script += FString().Printf(L"input s%d MovieRenders/center.0.%04d.png\n",i,count*captureConfig.step+captureConfig.step-1);    
            }
            
            count ++;
        }
        if(captureConfig.data.stereo){
            script += FString().Printf(L"output MovieRenders_output/%s.left.tif\n",*p.name);
        }else{
            // script += FString().Printf(L"output MovieRenders_output/%s.tif\n",*p.name);
            // if(captureConfig.jpg)
            //     script += FString().Printf(L"output MovieRenders_output/%s.jpg\n",*p.name);
            script += BuildOutputString(0,*p.name);
        }
        script += L"clear\n";
        if(captureConfig.data.stereo){
            for (int i = 0; i < captureConfig.data.total; i++)
            {
                if(captureConfig.IsFrontFace(i)){
                    script += FString().Printf(L"input s%d MovieRenders/center.0.%04d.png\n",i,count*captureConfig.step+captureConfig.step-1);
                }
                
                count ++;
            }
            script += FString().Printf(L"output MovieRenders_output/%s.right.tif\n",*p.name);
            script += L"clear\n";
        }
    }
    FString fPath = FPaths::Combine(captureConfig.outputdir,L"script.txt");
    FFileHelper::SaveStringToFile(script, *fPath);
    

    if(captureConfig.data.stereo){
        script = FString().Printf(L"width %d\nheight %d\n\n",captureConfig.data.outwidth,captureConfig.data.outheight*2);
        script += L"loop 0 1\n";
        for (FPanoPoint p : list->data){
            script += FString().Printf(L"background MovieRenders_output/%s.left.tif\n",*p.name);
            script += FString().Printf(L"background MovieRenders_output/%s.right.tif %d\n",*p.name,captureConfig.data.outheight);
            // script += FString().Printf(L"output MovieRenders_output/%s.jpg\n",*p.name);
            script += BuildOutputString(0,*p.name);
            script += L"clear\n";
        }
        script += L"endloop\n";

        fPath = FPaths::Combine(captureConfig.outputdir,L"scriptjoin.txt");
        FFileHelper::SaveStringToFile(script, *fPath);
        stitchcmd += FString().Printf(L"uestitch -script scriptjoin.txt\n");
    }

    if(captureConfig.autoStitch){
        cmd += FString().Printf(L"pushd \"%s\"\n",*captureConfig.outputdir);
        cmd += L"start stitch.cmd\n";
        cmd += L"popd\n";
    }
    if(captureConfig.clearImg)
        stitchcmd += L"rmdir MovieRenders /S /Q\n";
    fPath = FPaths::Combine(FPaths::ProjectDir(),L"render.cmd");
    FFileHelper::SaveStringToFile(cmd, *fPath);

    stitchcmd += L"popd\n";
    fPath = FPaths::Combine(captureConfig.outputdir,L"stitch.cmd");
    FFileHelper::SaveStringToFile(stitchcmd, *fPath);
}
APanoCapture* SPanoToolWidget::CheckMap(){
    UWorld* World = GEditor->GetEditorWorldContext().World();
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(World, APanoCapture::StaticClass(), FoundActors);
    
    if(FoundActors.Num()==0){
        UEditorDialogLibrary::ShowMessage(FText().FromString(L"Error"),FText().FromString(L"PanoCapture Actor can not found"),EAppMsgType::Ok);
        return nullptr;
    }
    if(FoundActors.Num()>1){
        UEditorDialogLibrary::ShowMessage(FText().FromString(L"Error"),FText().FromString(L"Have more one PanoCapture Actor in the scene"),EAppMsgType::Ok);
        return nullptr;
    }
    return (APanoCapture*)FoundActors[0];
}
void SPanoToolWidget::ResetSequence(){
    UMovieSceneSequence* captureSeq = Cast<ULevelSequence>(UEditorAssetLibrary::LoadAsset(L"/UEPanoTool/Animation/renderseqvideo"));
    UMovieScene* ms = captureSeq->GetMovieScene();
    UWorld* World = GEditor->GetEditorWorldContext().World();
    
    AActor* panocapture = CheckMap();
    if(!panocapture)
        return;

    FFrameRate DisplayRate = ms->GetDisplayRate();
    TRange<FFrameNumber> captureRange = ms->GetPlaybackRange();
    captureRange.SetLowerBound(TRangeBound<FFrameNumber>::Inclusive(ConvertFrameTime(0, DisplayRate, ms->GetTickResolution()).FrameNumber));
    captureRange.SetUpperBound(TRangeBound<FFrameNumber>::Exclusive(ConvertFrameTime(30, DisplayRate, ms->GetTickResolution()).FrameNumber));
    ms->SetPlaybackRange(captureRange);
    if(!captureSeq){
        return;
    }
    //Clear Seq
    if(ms->GetCameraCutTrack()){
        ms->RemoveCameraCutTrack();
    }
    TArray<UMovieSceneTrack*>tracks = ms->GetMasterTracks();
    for (UMovieSceneTrack* track: tracks){
        ms->RemoveMasterTrack(*track);
    }
    TArray<FMovieSceneBinding>binds = ms->GetBindings();
    for (FMovieSceneBinding bind: binds){
        if (!ms->RemovePossessable(bind.GetObjectGuid()))
		{
			ms->RemoveSpawnable(bind.GetObjectGuid());
		}
    }
    
    //End Clear Seq
    
    FGuid bindId = captureSeq->CreatePossessable(panocapture);
    
    FMovieSceneBinding* panobind = ms->FindBinding(bindId);
    UMovieSceneTrack* camtrack = ms->AddCameraCutTrack(UMovieSceneCameraCutTrack::StaticClass());
    UMovieSceneCameraCutSection* camSection = Cast<UMovieSceneCameraCutSection>(camtrack->CreateNewSection());
    camtrack->AddSection(*camSection);
    camSection->SetCameraBindingID(UE::MovieScene::FRelativeObjectBindingID(bindId));
    camSection->SetRange(captureRange);
    UEditorAssetLibrary::SaveAsset(L"/UEPanoTool/Animation/renderseqvideo");
    GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->OpenEditorForAsset(L"/UEPanoTool/Animation/renderseqvideo");
    
}

void SPanoToolWidget::GenSequence(){
    UMovieSceneSequence* captureSeq = Cast<ULevelSequence>(UEditorAssetLibrary::LoadAsset(L"/UEPanoTool/Animation/renderseq"));
    UMovieScene* ms = captureSeq->GetMovieScene();
    UWorld* World = GEditor->GetEditorWorldContext().World();
    FPanoPointList list = FPanoPointList();
    list.Scan(World);
    list.Save();
    SaveScriptFile(&list);
    AActor* panocapture = CheckMap();
    if(!panocapture)
        return;
    FFrameRate DisplayRate = ms->GetDisplayRate();
    TRange<FFrameNumber> captureRange = ms->GetPlaybackRange();
    captureConfig.endframe = list.data.Num()*captureConfig.step;
    captureRange.SetLowerBound(TRangeBound<FFrameNumber>::Inclusive(ConvertFrameTime(0, DisplayRate, ms->GetTickResolution()).FrameNumber));
    if(!captureConfig.data.stereo)
        captureRange.SetUpperBound(TRangeBound<FFrameNumber>::Exclusive(ConvertFrameTime(captureConfig.endframe*captureConfig.data.total, DisplayRate, ms->GetTickResolution()).FrameNumber));
    else
        captureRange.SetUpperBound(TRangeBound<FFrameNumber>::Exclusive(ConvertFrameTime(captureConfig.endframe*captureConfig.data.total*2, DisplayRate, ms->GetTickResolution()).FrameNumber));
    ms->SetPlaybackRange(captureRange);
    
    if(!captureSeq){
        return;
    }
    //Clear Seq
    // UMovieSceneSequenceExtensions
    // UMovieSceneBindingExtensions
    if(ms->GetCameraCutTrack()){
        ms->RemoveCameraCutTrack();
    }
    TArray<UMovieSceneTrack*>tracks = ms->GetMasterTracks();
    for (UMovieSceneTrack* track: tracks){
        ms->RemoveMasterTrack(*track);
    }
    TArray<FMovieSceneBinding>binds = ms->GetBindings();
    for (FMovieSceneBinding bind: binds){
        if (!ms->RemovePossessable(bind.GetObjectGuid()))
		{
			ms->RemoveSpawnable(bind.GetObjectGuid());
		}
    }
    
    //End Clear Seq
    
    FGuid bindId = captureSeq->CreatePossessable(panocapture);
    
    for (FMovieSceneBinding bind: ms->GetBindings()){
        if (bind.GetObjectGuid()==bindId)
		{
            UMovieScene3DTransformTrack* transformTrack = NewObject<UMovieScene3DTransformTrack>(ms,UMovieScene3DTransformTrack::StaticClass() , NAME_None, RF_Transactional);
			ms->AddGivenTrack(transformTrack,bindId);
            UMovieScene3DTransformSection* transformSection= Cast<UMovieScene3DTransformSection>(transformTrack->CreateNewSection());
            transformTrack->AddSection(*transformSection);
            transformSection->SetRange(captureRange);
            FMovieSceneChannelProxy& channelProxy  = transformSection->GetChannelProxy();
            TArrayView <FMovieSceneDoubleChannel*> channels = channelProxy.GetChannels<FMovieSceneDoubleChannel>();
            int count = 0;
            for (FPanoPoint p : list.data)
            {
                for (int i = 0; i < captureConfig.data.total; i++)
                {
                    FQuat q = FQuat();
                    FVector pos = captureConfig.data.GetPos(i)+p.pos;
                    FVector rot = captureConfig.data.GetRotate(i);
                    q = FQuat().MakeFromEuler(rot);
                    if(p.capturerot){
                        q = FQuat().MakeFromEuler(p.rot)*q;
                    }
                    double roll = q.Euler().X;
                    double pitch = q.Euler().Y;
                    double yaw = q.Euler().Z;
                    FFrameNumber f = ConvertFrameTime(count*captureConfig.step, DisplayRate, ms->GetTickResolution()).FrameNumber;
                    channels[0]->AddConstantKey(f,pos.X);
                    channels[1]->AddConstantKey(f,pos.Y);
                    channels[2]->AddConstantKey(f,pos.Z);
                    channels[3]->AddConstantKey(f,roll);
                    channels[4]->AddConstantKey(f,pitch);
                    channels[5]->AddConstantKey(f,yaw);
                    count++;
                }
                if(captureConfig.data.stereo){
                    for (int i = 0; i < captureConfig.data.total; i++)
                    {
                        FQuat q = FQuat();
                        FVector pos = captureConfig.data.GetPos(i+captureConfig.data.total)+p.pos;
                        FVector rot = captureConfig.data.GetRotate(i+captureConfig.data.total);
                        q = FQuat().MakeFromEuler(rot);
                        if(p.capturerot){
                            q = FQuat().MakeFromEuler(p.rot)*q;
                        }
                        double roll = q.Euler().X;
                        double pitch = q.Euler().Y;
                        double yaw = q.Euler().Z;
                        FFrameNumber f = ConvertFrameTime(count*captureConfig.step, DisplayRate, ms->GetTickResolution()).FrameNumber;
                        channels[0]->AddConstantKey(f,pos.X);
                        channels[1]->AddConstantKey(f,pos.Y);
                        channels[2]->AddConstantKey(f,pos.Z);
                        channels[3]->AddConstantKey(f,roll);
                        channels[4]->AddConstantKey(f,pitch);
                        channels[5]->AddConstantKey(f,yaw);
                        count++;
                    }
                }
                
            }
		}
    }
    
    FMovieSceneBinding* panobind = ms->FindBinding(bindId);
    UMovieSceneTrack* camtrack = ms->AddCameraCutTrack(UMovieSceneCameraCutTrack::StaticClass());
    UMovieSceneCameraCutSection* camSection = Cast<UMovieSceneCameraCutSection>(camtrack->CreateNewSection());
    camtrack->AddSection(*camSection);
    camSection->SetCameraBindingID(UE::MovieScene::FRelativeObjectBindingID(bindId));
    camSection->SetRange(captureRange);
    
    UEditorAssetLibrary::SaveAsset(L"/UEPanoTool/Animation/renderseq");
}
#undef LOCTEXT_NAMESPACE