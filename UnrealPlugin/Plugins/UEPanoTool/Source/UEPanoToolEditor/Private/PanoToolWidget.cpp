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
    outputWidth = captureConfig.data.outwidth; 
    step = captureConfig.step;
    int CaptureStyleSelectedIndex = 0;
    for (size_t i = 0; i < captureStyleData.data.Num(); i++)
    {
        if(captureStyleData.data[i].name==captureConfig.data.name){
            CaptureStyleSelectedIndex = i;
        }
    }
    captureConfig.data = captureStyleData.data[CaptureStyleSelectedIndex];
    CaptureStyleStrings.Empty();
    
    
    CaptureStyleStrings.Add(MakeShareable(new FString(LOCTEXT("PanoTool", "mono6").ToString())));
    CaptureStyleStrings.Add(MakeShareable(new FString(LOCTEXT("PanoTool", "mono26").ToString())));
    CaptureStyleStrings.Add(MakeShareable(new FString(LOCTEXT("PanoTool", "mono36").ToString())));
    CaptureStyleStrings.Add(MakeShareable(new FString(LOCTEXT("PanoTool", "stereo240").ToString())));
    CaptureStyleStrings.Add(MakeShareable(new FString(LOCTEXT("PanoTool", "stereo2160").ToString())));
    
    
    
    OutputStrings.Empty();
    OutputStrings.Add(MakeShareable(new FString(LOCTEXT("PanoTool", "EquirectAngular").ToString())));
    OutputStrings.Add(MakeShareable(new FString(LOCTEXT("PanoTool", "Cube").ToString())));
    OutputStrings.Add(MakeShareable(new FString(LOCTEXT("PanoTool", "Google EAC").ToString())));
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
                .Value(outputWidth)
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
                .Text( LOCTEXT("PanoTool", "Step"))
            ]
            +SHorizontalBox::Slot()
            .FillWidth(2)
            [
                SNew(SSpinBox<int>)
                .MinValue(2)
                .MaxValue(10)
                .Value(step)
                .OnValueChanged(this,&SPanoToolWidget::OnChangeStep)
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
    outputWidth = width;
}
void SPanoToolWidget::OnChangeStep(int s){
    step = s;
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
    captureConfig.data.outwidth = outputWidth;
    captureConfig.data.outheight = outputWidth/2;
    captureConfig.step = step;
    float outrad = 3.1416f;
    if(captureConfig.projector==3 || captureConfig.projector==4){
        captureConfig.data.outheight = outputWidth;
        outrad *= 0.5;
    }
    int w = outputWidth*FMath::Tan(FMath::DegreesToRadians(captureConfig.data.fov)*0.5f)/outrad;
    captureConfig.data.width = w-w%captureConfig.data.basewidth+captureConfig.data.basewidth;
    captureConfig.data.height = captureConfig.data.baseheight*captureConfig.data.width/captureConfig.data.basewidth;
    UWorld* World = GEditor->GetEditorWorldContext().World();
    FString lname = World->GetCurrentLevel()->GetPathName();
    FString fname = "";
    FString sname = "";
    lname.Split(L".",&fname,&sname);
    captureConfig.map = fname;
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
    return FString().Printf(L"%s %s %s %s -renderpano -renderindex %d\n",*captureConfig.ued,*captureConfig.project,*captureConfig.map,*renderArg,index);
}
void SPanoToolWidget::SaveVideoScriptFile(){
    FString cmd = L"mkdir Saved\\MovieRenders_output\n";

    FString script = FString().Printf(L"width %d\nheight %d\nprojector %d\n",captureConfig.data.outwidth,captureConfig.data.outheight,captureConfig.projector);
    for (int i = 0; i < captureConfig.data.total; i++){
        FVector r = captureConfig.data.GetRotate(i);
        script += FString().Printf(L"plane s%d %f %f 0 %f %d %d\n",i,r.Z,r.Y,captureConfig.data.fov,captureConfig.data.width,captureConfig.data.height);
    }
    UMovieSceneSequence* captureSeq = Cast<ULevelSequence>(UEditorAssetLibrary::LoadAsset(L"/UEPanoTool/Animation/renderseq"));
    UMovieScene* ms = captureSeq->GetMovieScene();
    FFrameRate DisplayRate = ms->GetDisplayRate();
    captureConfig.startframe = ConvertFrameTime(UE::MovieScene::DiscreteInclusiveLower(ms->GetPlaybackRange()), ms->GetTickResolution(), DisplayRate).FloorToFrame().Value;
    captureConfig.endframe = ConvertFrameTime(UE::MovieScene::DiscreteExclusiveUpper(ms->GetPlaybackRange()), ms->GetTickResolution(), DisplayRate).FloorToFrame().Value;
    
    script += FString().Printf(L"loop %d %d\n",captureConfig.startframe,captureConfig.endframe);
    for (int i = 0; i < captureConfig.data.total; i++){
        cmd += RenderCmd(i);
        script += FString().Printf(L"input s%d Saved/MovieRenders/%s.%d.%%04d.png\n",i,*captureConfig.data.GetEyeName(i),i);
    }
    script += FString().Printf(L"output Saved/MovieRenders_output/%s.%%04d.tif\n",*captureConfig.data.GetEyeName(0));
    script += FString().Printf(L"output Saved/MovieRenders_output/%s.%%04d.jpg\n",*captureConfig.data.GetEyeName(0));
    script += FString().Printf(L"clear\n");
    if(captureConfig.data.stereo){
        for (int i = 0; i < captureConfig.data.total; i++){
            cmd += RenderCmd(i+captureConfig.data.total);
            script += FString().Printf(L"input s%d Saved/MovieRenders/%s.%d.%%04d.png\n",i,*captureConfig.data.GetEyeName(i+captureConfig.data.total),i);
        }
        script += FString().Printf(L"output Saved/MovieRenders_output/right.%%04d.tif\n");
        script += FString().Printf(L"output Saved/MovieRenders_output/right.%%04d.jpg\n");
        script += FString().Printf(L"clear\n");
    }
    script += FString().Printf(L"endloop\n");
    

    FString fPath = FPaths::Combine(FPaths::ProjectPluginsDir(),L"UEPanoTool/Tool/script.txt");
    FFileHelper::SaveStringToFile(script, *fPath);

    cmd += FString().Printf(L".\\Plugins\\UEPanoTool\\Tool\\uestitch -script .\\Plugins\\UEPanoTool\\Tool\\script.txt\n");
    if(captureConfig.data.stereo){
        script = FString().Printf(L"width %d\nheight %d\n\n",captureConfig.data.outwidth,captureConfig.data.outwidth);
        script += FString().Printf(L"loop %d %d\n",captureConfig.startframe,captureConfig.endframe);
        script += FString().Printf(L"background Saved/MovieRenders_output/left.%%04d.tif\n");
        script += FString().Printf(L"background Saved/MovieRenders_output/right.%%04d.tif %d\n",captureConfig.data.outheight);
        script += FString().Printf(L"output Saved/MovieRenders_output/%%04d.jpg\n");
        script += L"clear\n";
        script += L"endloop\n";
        fPath = FPaths::Combine(FPaths::ProjectPluginsDir(),L"UEPanoTool/Tool/scriptjoin.txt");
        FFileHelper::SaveStringToFile(script, *fPath);
        cmd += FString().Printf(L".\\Plugins\\UEPanoTool\\Tool\\uestitch -script .\\Plugins\\UEPanoTool\\Tool\\scriptjoin.txt\n");
    }
    cmd += L"explorer Saved\\MovieRenders_output\n";
    fPath = FPaths::Combine(FPaths::ProjectDir(),L"render.cmd");
    FFileHelper::SaveStringToFile(cmd, *fPath);
}

void SPanoToolWidget::SaveScriptFile(FPanoPointList* list){
    FString cmd = L"mkdir Saved\\MovieRenders_output\n";
    cmd += RenderCmd(0);
    cmd += FString().Printf(L".\\Plugins\\UEPanoTool\\Tool\\uestitch -script .\\Plugins\\UEPanoTool\\Tool\\script.txt\n");
    

    FString script = FString().Printf(L"width %d\nheight %d\nprojector %d\n",captureConfig.data.outwidth,captureConfig.data.outheight,captureConfig.projector);
    for (int i = 0; i < captureConfig.data.total; i++){
        FVector r = captureConfig.data.GetRotate(i);
        script += FString().Printf(L"plane s%d %f %f 0 %f %d %d\n",i,r.Z,r.Y,captureConfig.data.fov,captureConfig.data.width,captureConfig.data.height);
    }
    int count = 0;
    for (FPanoPoint p : list->data){
        for (int i = 0; i < captureConfig.data.total; i++)
        {
            script += FString().Printf(L"input s%d Saved/MovieRenders/center.0.%04d.png\n",i,count*captureConfig.step+captureConfig.step-1);
            count ++;
        }
        if(captureConfig.data.stereo){
            script += FString().Printf(L"output Saved/MovieRenders_output/%s.left.tif\n",*p.name);
        }else{
            script += FString().Printf(L"output Saved/MovieRenders_output/%s.tif\n",*p.name);
            script += FString().Printf(L"output Saved/MovieRenders_output/%s.jpg\n",*p.name);
        }
        script += L"clear\n";
        if(captureConfig.data.stereo){
            for (int i = 0; i < captureConfig.data.total; i++)
            {
                script += FString().Printf(L"input s%d Saved/MovieRenders/center.0.%04d.png\n",i,count*captureConfig.step+captureConfig.step-1);
                count ++;
            }
            script += FString().Printf(L"output Saved/MovieRenders_output/%s.right.tif\n",*p.name);
            script += L"clear\n";
        }
    }
    FString fPath = FPaths::Combine(FPaths::ProjectPluginsDir(),L"UEPanoTool/Tool/script.txt");
    FFileHelper::SaveStringToFile(script, *fPath);

    if(captureConfig.data.stereo){
        script = FString().Printf(L"width %d\nheight %d\n\n",captureConfig.data.outwidth,captureConfig.data.outwidth);
        script += L"loop 0 1\n";
        for (FPanoPoint p : list->data){
            script += FString().Printf(L"background Saved/MovieRenders_output/%s.left.tif\n",*p.name);
            script += FString().Printf(L"background Saved/MovieRenders_output/%s.right.tif %d\n",*p.name,captureConfig.data.outheight);
            script += FString().Printf(L"output Saved/MovieRenders_output/%s.jpg\n",*p.name);
            script += L"clear\n";
        }
        script += L"endloop\n";

        fPath = FPaths::Combine(FPaths::ProjectPluginsDir(),L"UEPanoTool/Tool/scriptjoin.txt");
        FFileHelper::SaveStringToFile(script, *fPath);
        cmd += FString().Printf(L".\\Plugins\\UEPanoTool\\Tool\\uestitch -script .\\Plugins\\UEPanoTool\\Tool\\scriptjoin.txt\n");
    }
    

    cmd += L"explorer Saved\\MovieRenders_output\n";
    fPath = FPaths::Combine(FPaths::ProjectDir(),L"render.cmd");
    FFileHelper::SaveStringToFile(cmd, *fPath);
    
}

void SPanoToolWidget::ResetSequence(){
    UMovieSceneSequence* captureSeq = Cast<ULevelSequence>(UEditorAssetLibrary::LoadAsset(L"/UEPanoTool/Animation/renderseqvideo"));
    UMovieScene* ms = captureSeq->GetMovieScene();
    UWorld* World = GEditor->GetEditorWorldContext().World();
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(World, APanoCapture::StaticClass(), FoundActors);
    
    if(FoundActors.Num()==0){
        UEditorDialogLibrary::ShowMessage(FText().FromString(L"Error"),FText().FromString(L"PanoCapture Actor can not found"),EAppMsgType::Ok);
        return;
    }
    if(FoundActors.Num()>1){
        UEditorDialogLibrary::ShowMessage(FText().FromString(L"Error"),FText().FromString(L"Have more one PanoCapture Actor in the scene"),EAppMsgType::Ok);
        return;
    }
    AActor* panocapture = FoundActors[0];
    FFrameRate DisplayRate = ms->GetDisplayRate();
    TRange<FFrameNumber> captureRange = ms->GetPlaybackRange();
    captureRange.SetLowerBound(TRangeBound<FFrameNumber>::Inclusive(ConvertFrameTime(0, DisplayRate, ms->GetTickResolution()).FrameNumber));
    captureRange.SetUpperBound(TRangeBound<FFrameNumber>::Exclusive(ConvertFrameTime(30, DisplayRate, ms->GetTickResolution()).FrameNumber));
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
    
    FMovieSceneBinding* panobind = ms->FindBinding(bindId);
    UMovieSceneTrack* camtrack = ms->AddCameraCutTrack(UMovieSceneCameraCutTrack::StaticClass());
    UMovieSceneCameraCutSection* camSection = Cast<UMovieSceneCameraCutSection>(camtrack->CreateNewSection());
    camtrack->AddSection(*camSection);
    camSection->SetCameraBindingID(UE::MovieScene::FRelativeObjectBindingID(bindId));
    camSection->SetRange(captureRange);
    UEditorAssetLibrary::SaveAsset(L"/UEPanoTool/Animation/renderseqvideo");
    
}

void SPanoToolWidget::GenSequence(){
    UMovieSceneSequence* captureSeq = Cast<ULevelSequence>(UEditorAssetLibrary::LoadAsset(L"/UEPanoTool/Animation/renderseq"));
    UMovieScene* ms = captureSeq->GetMovieScene();
    UWorld* World = GEditor->GetEditorWorldContext().World();
    FPanoPointList list = FPanoPointList();
    list.Scan(World);
    list.Save();
    SaveScriptFile(&list);
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(World, APanoCapture::StaticClass(), FoundActors);
    
    if(FoundActors.Num()==0){
        UEditorDialogLibrary::ShowMessage(FText().FromString(L"Error"),FText().FromString(L"PanoCapture Actor can not found"),EAppMsgType::Ok);
        return;
    }
    if(FoundActors.Num()>1){
        UEditorDialogLibrary::ShowMessage(FText().FromString(L"Error"),FText().FromString(L"Have more one PanoCapture Actor in the scene"),EAppMsgType::Ok);
        return;
    }
    AActor* panocapture = FoundActors[0];
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