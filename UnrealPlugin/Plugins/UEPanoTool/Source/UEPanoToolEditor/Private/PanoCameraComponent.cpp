#include "PanoCameraComponent.h"
#include "MoviePipelineQueue.h"
#include "EditorAssetLibrary.h"
#include "EditorDialogLibrary.h"
#include "UObject/ConstructorHelpers.h"
// #include "MoviePipelineMasterConfig.h"
#include "MoviePipelineOutputSetting.h"
#include "MoviePipelineConsoleVariableSetting.h"
#include "MoviePipelineDeferredPasses.h"
#include "Misc/FileHelper.h"
#include "JsonObjectConverter.h"
#include "Camera/CameraComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/CameraActor.h"

UPanoCameraComponent::UPanoCameraComponent(){
#if ENGINE_MINOR_VERSION == 1
    static ConstructorHelpers::FObjectFinder<UMoviePipelineMasterConfig> renderconfig(TEXT("/UEPanoTool/Animation/renderconfigvideo"));
	moviePipelineMasterConfig = renderconfig.Object;
#else //2
    static ConstructorHelpers::FObjectFinder<UMoviePipelinePrimaryConfig> renderconfig(TEXT("/UEPanoTool/Animation/renderconfigvideo"));
	moviePipelineMasterConfig = renderconfig.Object;
#endif
    
    static ConstructorHelpers::FObjectFinder<ULevelSequence> renderseq(TEXT("/UEPanoTool/Animation/zero"));
	zeroSequence = renderseq.Object;

    FString str;
    FString fPath = FPaths::Combine(FPaths::ProjectPluginsDir(), TEXT("UEPanoTool/Content/capturestyle.json"));
    
    FFileHelper::LoadFileToString(captureStyleDataText, *fPath);
    

    //=========add pano.RenderIndex if not exist
	pCvar = IConsoleManager::Get().FindConsoleVariable(TEXT("pano.RenderIndex")); 
	if(!pCvar){
		IConsoleManager::Get().RegisterConsoleVariable(L"pano.RenderIndex",
		0,
		L"RenderIndex control camera angle",
		ECVF_Scalability | ECVF_RenderThreadSafe);
		pCvar = IConsoleManager::Get().FindConsoleVariable(TEXT("pano.RenderIndex")); 
        pCvar->OnChangedDelegate().AddStatic(UPanoCameraComponent::OnChangeRenderIndex);
	}
	//=========end add cvar
}
void UPanoCameraComponent::OnChangeRenderIndex(IConsoleVariable * val){
    UE_LOG(LogTemp, Log,L"RenderIndex==============>%d",val->GetInt());
    UWorld* World = GEditor->GetEditorWorldContext().World();
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(World, ACameraActor::StaticClass(), FoundActors);
    for(AActor* cam :FoundActors){
        UPanoCameraComponent *panocam = Cast<UPanoCameraComponent>(cam->GetComponentByClass(UPanoCameraComponent::StaticClass()));
        if(panocam){
            panocam->renderIndex = val->GetInt();
            panocam->TestRenderCameraView();
        }
    }
}
void UPanoCameraComponent::BeginPlay(){
    Super::BeginPlay();
    UE_LOG(LogTemp, Log,L"BeginPlay==============>");
    // TestRenderCameraView();
}
void UPanoCameraComponent::TestRenderCameraView(){
    AActor* actor = this->GetAttachmentRootActor();
    CameraComponent = Cast<UCameraComponent>(this->GetAttachmentRootActor()->GetComponentByClass(UCameraComponent::StaticClass()));
    if(!CameraComponent){
        //UEditorDialogLibrary::ShowMessage(FText().FromString(L"Error"),FText().FromString(L"PanoCameraComponent need Attach to camera"),EAppMsgType::Ok);
        return;
    }
	CameraComponent->bConstrainAspectRatio = false;
    CameraComponent->PostProcessSettings.AutoExposureMethod = EAutoExposureMethod::AEM_Manual;
    CameraComponent->PostProcessSettings.AutoExposureBias = 10;
	CameraComponent->PostProcessSettings.VignetteIntensity = 0;
	CameraComponent->PostProcessSettings.MotionBlurAmount = 0;
	CameraComponent->PostProcessSettings.LensFlareIntensity = 0;
	CameraComponent->PostProcessSettings.BloomIntensity = 0.01;
	CameraComponent->PostProcessSettings.FilmGrainIntensity = 0.0f;
	CameraComponent->PostProcessSettings.ScreenSpaceReflectionIntensity = 0.0f;
	CameraComponent->PostProcessSettings.bOverride_VignetteIntensity = true;
    CameraComponent->PostProcessSettings.bOverride_AutoExposureMethod = true;
    CameraComponent->PostProcessSettings.bOverride_AutoExposureBias = true;
	CameraComponent->PostProcessSettings.bOverride_VignetteIntensity = true;
	CameraComponent->PostProcessSettings.bOverride_MotionBlurAmount = true;
	CameraComponent->PostProcessSettings.bOverride_LensFlareIntensity = true;
	CameraComponent->PostProcessSettings.bOverride_BloomIntensity = true;
	CameraComponent->PostProcessSettings.bOverride_FilmGrainIntensity = true;

    SetupConfig();

    int face = renderIndex;
    FQuat rot = FQuat();
	FVector offset = FVector(0);
	FString campos = L"center";
	int facenum = face;
    campos = captureConfig->data.GetEyeName(face);
    offset = captureConfig->data.GetPos(face);
    
    // FVector oldr = CameraComponent->GetRelativeRotation().Euler();
    // UE_LOG(LogTemp, Log,L"RenderIndex==============>%f,%f,%f",oldr.X,oldr.Y,oldr.Z);
    // FVector r = captureConfig->data.GetRotate(face);
    // UE_LOG(LogTemp, Log,L"RenderIndex==============>%f,%f,%f",r.X,r.Y,r.Z);

    rot = FQuat().MakeFromEuler(captureConfig->data.GetRotate(face));
	CameraComponent->SetRelativeRotation(rot);
	CameraComponent->SetRelativeLocation(offset);
	CameraComponent->FieldOfView = captureConfig->data.fov;

}
void UPanoCameraComponent::CreateMRQ(){
    if(!mrq || !sequence){
        if(!mrq)
            UEditorDialogLibrary::ShowMessage(FText().FromString(L"Error"),FText().FromString(L"mrq can not found"),EAppMsgType::Ok);
        else if(!sequence)
            UEditorDialogLibrary::ShowMessage(FText().FromString(L"Error"),FText().FromString(L"sequence can not found"),EAppMsgType::Ok);
        return;
    }
    mrq->DeleteAllJobs();
    SetupConfig();
    

    UWorld* World = GEditor->GetEditorWorldContext().World();
    for (size_t i = 0; i <= captureConfig->data.total; i++)
    {
        UMoviePipelineExecutorJob* job = mrq->AllocateNewJob(UMoviePipelineExecutorJob::StaticClass());    
        job->JobName = FString().Printf(L"phase (%d/%d)",i,captureConfig->data.total);
        FString lname = World->GetCurrentLevel()->GetPathName();
        FString fname = "";
        FString sname = "";
        lname.Split(L":",&fname,&sname);
        job->Map = fname;
        if(i==0){
            job->Sequence = zeroSequence;
        }else{
            job->Sequence = sequence;
        }
        job->Author = "UEPanoTool";
        job->Comment =L"Automatically generated using UEPanoTool";
#if ENGINE_MINOR_VERSION == 1
    UMoviePipelineMasterConfig* config = job->GetConfiguration();
#else //2
    UMoviePipelinePrimaryConfig* config = job->GetConfiguration();
#endif        
        
        config->CopyFrom(moviePipelineMasterConfig);
        config->DisplayName = FString().Printf(L"config_%d",i);
        UMoviePipelineConsoleVariableSetting* cmdSetting = Cast<UMoviePipelineConsoleVariableSetting>(config->FindOrAddSettingByClass(UMoviePipelineConsoleVariableSetting::StaticClass()));
        
        cmdSetting->EndConsoleCommands.Empty();
        cmdSetting->EndConsoleCommands.Add(FString().Printf(L"pano.RenderIndex %d",i));

        UMoviePipelineOutputSetting* outputsetting = Cast<UMoviePipelineOutputSetting>(config->FindOrAddSettingByClass(UMoviePipelineOutputSetting::StaticClass()));
        outputsetting->FileNameFormat = FString().Printf(L"center.%d.{frame_number}",i-1);
        outputsetting->OutputResolution = FIntPoint(captureConfig->data.width,captureConfig->data.height);
        outputsetting->OutputDirectory.Path = FPaths::Combine(captureConfig->outputdir,L"MovieRenders");
    }
    FString p = mrq->GetOutermost()->GetPathName();
    UEditorAssetLibrary::SaveAsset(p);
    GenStitch();
    GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->OpenEditorForAsset(p);
    
}
void UPanoCameraComponent::GenStitch(){
    FString stitchcmd = FString("pushd \"%~dp0\"\nmkdir MovieRenders_output\n");
    FString script = FString().Printf(L"width %d\nheight %d\nprojector %d\n",captureConfig->data.outwidth,captureConfig->data.outheight,captureConfig->projector);
    for (size_t i = 0; i < captureConfig->data.total; i++){
        FVector r = captureConfig->data.GetRotate(i);
        if(!captureConfig->IsFrontFace(i)){
            continue;
        }
        script += FString().Printf(L"plane s%d %f %f 0 %f %d %d\n",i,r.Z,r.Y,captureConfig->data.fov,captureConfig->data.width,captureConfig->data.height);
    }
    TArray<FString> ls = GetLayerObj();
    script += FString().Printf(L"loop %d %d\n",captureConfig->startframe,captureConfig->endframe);

    for (FString l : ls)
    {
        
        for (int i = 0; i < captureConfig->data.total; i++){
            if(!captureConfig->IsFrontFace(i)){
                continue;
            }
            script += FString().Printf(L"input s%d MovieRenders/%s.%d%s.%%04d.png\n",i,*captureConfig->data.GetEyeName(i),i,*l);
        }
        script += BuildOutputString(0,l);
        script += FString().Printf(L"clear\n");
        if(captureConfig->data.stereo){
            for (int i = 0; i < captureConfig->data.total; i++){
                if(!captureConfig->IsFrontFace(i)){
                    continue;
                }
                script += FString().Printf(L"input s%d MovieRenders/%s.%d%s.%%04d.png\n",i,*captureConfig->data.GetEyeName(i+captureConfig->data.total),i,*l);
            }
            script += BuildOutputString(1,l);
            script += FString().Printf(L"clear\n");
        }
    }
    
    script += FString().Printf(L"endloop\n");
    FString fPath = FPaths::Combine(captureConfig->outputdir,L"script.txt");
    FFileHelper::SaveStringToFile(script, *fPath);

    stitchcmd += FString().Printf(L"uestitch -script script.txt\n");
    if(captureConfig->data.stereo){
        script = FString().Printf(L"width %d\nheight %d\n\n",captureConfig->data.outwidth,captureConfig->data.outheight*2);
        script += FString().Printf(L"loop %d %d\n",captureConfig->startframe,captureConfig->endframe);
        script += FString().Printf(L"background MovieRenders_output/left.%%04d.tif\n");
        script += FString().Printf(L"background MovieRenders_output/right.%%04d.tif %d\n",captureConfig->data.outheight);
        if(jpg)
            script += FString().Printf(L"output MovieRenders_output/%%04d.jpg\n");
        script += L"clear\n";
        script += L"endloop\n";
        fPath = FPaths::Combine(captureConfig->outputdir,L"scriptjoin.txt");
        FFileHelper::SaveStringToFile(script, *fPath);
        stitchcmd += FString().Printf(L"uestitch -script scriptjoin.txt\n");
    }
    stitchcmd += L"popd\n";
    fPath = FPaths::Combine(captureConfig->outputdir,L"stitch.cmd");
    FFileHelper::SaveStringToFile(stitchcmd, *fPath);
    fPath = FPaths::Combine(FPaths::ProjectPluginsDir(),L"/UEPanoTool/Tool/uestitch.exe");
    fPath = L"copy \""+ fPath + L"\" \"" + captureConfig->outputdir + L"\"";
    fPath = fPath.Replace(L"/",L"\\") + L" /y";
    int ret = _tsystem(*fPath);
}
FString UPanoCameraComponent::BuildOutputString(int eye,FString layer){
    FString script = FString();
    if(tiff){
        script += FString().Printf(L"output MovieRenders_output/%s%s.%%04d.tif\n",*captureConfig->data.GetEyeName(0),*layer);
    }
    if(png){
        script += FString().Printf(L"output MovieRenders_output/%s%s.%%04d.png\n",*captureConfig->data.GetEyeName(0),*layer);
    }
    if(jpg){
        script += FString().Printf(L"output MovieRenders_output/%s%s.%%04d.jpg\n",*captureConfig->data.GetEyeName(0),*layer);
    }
    if(!tiff && !png && !jpg){
        script += FString().Printf(L"output MovieRenders_output/%s%s.%%04d.jpg\n",*captureConfig->data.GetEyeName(0),*layer);
    }
    return script;
}
TArray<FString> UPanoCameraComponent::GetLayerObj(){
    TArray<FString> ls = TArray<FString>();
	UMoviePipelineDeferredPassBase * renderPassSetting = Cast<UMoviePipelineDeferredPassBase >(moviePipelineMasterConfig->FindOrAddSettingByClass(UMoviePipelineDeferredPassBase ::StaticClass()));
	
	for(FActorLayer l:renderPassSetting->ActorLayers){
		ls.Add(L".FinalImage" + l.Name.ToString());
	}
	if(!renderPassSetting->ActorLayers.Num())
		ls.Add(L"");
	else{
        if(renderPassSetting->bAddDefaultLayer){
            ls.Add(L".FinalImageDefaultLayer");
        }
        if(renderPassSetting->bRenderMainPass){
            ls.Add(L".FinalImage");
        }
    }
	
	return ls;
}
void UPanoCameraComponent::SetupConfig(){
    
    if(!captureConfig)
        captureConfig = new FPanoCaptureConfig();
    
    FPanoCaptureStyleData captureStyleData = FPanoCaptureStyleData();
    FJsonObjectConverter::JsonObjectStringToUStruct(captureStyleDataText,&captureStyleData);
    captureConfig->data = captureStyleData.data[captureStyle];
    captureConfig->projector = outputStyle;
    captureConfig->data.outwidth = outputWidth;
    captureConfig->tiff = tiff;
    captureConfig->jpg = jpg;
    captureConfig->png = png;
    if(outputDiretory.Len())
        captureConfig->outputdir = outputDiretory;
    else{
        captureConfig->outputdir = FPaths::Combine(FPaths::ProjectDir(),L"Saved");
    }
    


    captureConfig->data.outheight = captureConfig->data.outwidth/2;
    float outrad = 3.1416f;
    if(captureConfig->projector==3 || captureConfig->projector==4){
        captureConfig->data.outheight = captureConfig->data.outwidth;
        outrad *= 0.5;
    }
    int w = captureConfig->data.outwidth*FMath::Tan(FMath::DegreesToRadians(captureConfig->data.fov)*0.5f)/outrad;
    captureConfig->data.width = w-w%captureConfig->data.basewidth+captureConfig->data.basewidth;
    captureConfig->data.height = captureConfig->data.baseheight*captureConfig->data.width/captureConfig->data.basewidth;

    UMovieScene* ms = sequence->GetMovieScene();
    FFrameRate DisplayRate = ms->GetDisplayRate();
    captureConfig->startframe = ConvertFrameTime(UE::MovieScene::DiscreteInclusiveLower(ms->GetPlaybackRange()), ms->GetTickResolution(), DisplayRate).FloorToFrame().Value;
    captureConfig->endframe = ConvertFrameTime(UE::MovieScene::DiscreteExclusiveUpper(ms->GetPlaybackRange()), ms->GetTickResolution(), DisplayRate).FloorToFrame().Value;
    
		
}