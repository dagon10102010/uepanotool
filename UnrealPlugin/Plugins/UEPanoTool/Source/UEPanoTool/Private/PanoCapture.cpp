#include "PanoCapture.h"
#include "Camera/CameraComponent.h"
#include "PanoToolData.h"
#include "MoviePipelineQueueEngineSubsystem.h"
#include "UObject/ConstructorHelpers.h"
#include "MoviePipelineOutputSetting.h"
#include "Kismet/GameplayStatics.h"
#include "MoviePipelineRenderPass.h"
#include "MoviePipelineDeferredPasses.h"
// Fill out your copyright notice in the Description page of Project Settings.
// Sets default values
APanoCapture::APanoCapture()
{
	// SceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SceneComponent"));
	// SetRootComponent(SceneComponent);
    // CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComponent"));
	// CameraComponent->AttachToComponent(SceneComponent, FAttachmentTransformRules::KeepRelativeTransform);
	CameraComponent = this->GetCameraComponent();
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
	
	

 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	// PrimaryActorTick.bCanEverTick = true;
	// PrimaryActorTick.bTickEvenWhenPaused = true;
	captureConfig = new FPanoCaptureConfig();
	
	static ConstructorHelpers::FObjectFinder<ULevelSequence> renderseq(TEXT("/UEPanoTool/Animation/renderseq"));
	levelSequence = renderseq.Object;
	static ConstructorHelpers::FObjectFinder<ULevelSequence> renderseqvideo(TEXT("/UEPanoTool/Animation/renderseqvideo"));
	levelSequenceVideo = renderseqvideo.Object;
#if ENGINE_MINOR_VERSION == 1
    static ConstructorHelpers::FObjectFinder<UMoviePipelineMasterConfig> renderconfig(TEXT("/UEPanoTool/Animation/renderconfig"));
	moviePipelineMasterConfig = renderconfig.Object;
	static ConstructorHelpers::FObjectFinder<UMoviePipelineMasterConfig> renderconfigvideo(TEXT("/UEPanoTool/Animation/renderconfigvideo"));
	moviePipelineMasterConfigVideo = renderconfigvideo.Object;

	static ConstructorHelpers::FObjectFinder<UMoviePipelineMasterConfig> renderconfigPathTracer(TEXT("/UEPanoTool/Animation/renderconfigPathTracer"));
	moviePipelineMasterConfigPathTracer = renderconfigPathTracer.Object;
	static ConstructorHelpers::FObjectFinder<UMoviePipelineMasterConfig> renderconfigvideoPathTracer(TEXT("/UEPanoTool/Animation/renderconfigvideoPathTracer"));
	moviePipelineMasterConfigVideoPathTracer = renderconfigvideoPathTracer.Object;

#else //2
	static ConstructorHelpers::FObjectFinder<UMoviePipelinePrimaryConfig> renderconfig(TEXT("/UEPanoTool/Animation/renderconfig"));
	moviePipelineMasterConfig = renderconfig.Object;
	static ConstructorHelpers::FObjectFinder<UMoviePipelinePrimaryConfig> renderconfigvideo(TEXT("/UEPanoTool/Animation/renderconfigvideo"));
	moviePipelineMasterConfigVideo = renderconfigvideo.Object;

	static ConstructorHelpers::FObjectFinder<UMoviePipelinePrimaryConfig> renderconfigPathTracer(TEXT("/UEPanoTool/Animation/renderconfigPathTracer"));
	moviePipelineMasterConfigPathTracer = renderconfigPathTracer.Object;
	static ConstructorHelpers::FObjectFinder<UMoviePipelinePrimaryConfig> renderconfigvideoPathTracer(TEXT("/UEPanoTool/Animation/renderconfigvideoPathTracer"));
	moviePipelineMasterConfigVideoPathTracer = renderconfigvideoPathTracer.Object;
#endif
	
}

void APanoCapture::BeginPlay()
{
	Super::BeginPlay();
    FString ParameterValue;
	
    captureConfig->Load();
	if (FParse::Value(FCommandLine::Get(), TEXT("renderpano"), ParameterValue))
	{
		
		if (FParse::Value(FCommandLine::Get(), TEXT("renderindex"), renderindex))
		{
			SetupCamera(renderindex);
			RunRender();
		}
	}
}
void APanoCapture::SetupCamera(int face){
	FQuat rot = FQuat();
	FVector offset = FVector(0);
	campos = L"center";
	facenum = face;
	if(captureConfig->video){
		campos = captureConfig->data.GetEyeName(face);
		offset = captureConfig->data.GetPos(face);
		rot = FQuat().MakeFromEuler(captureConfig->data.GetRotate(face));
	}
	CameraComponent->SetRelativeRotation(rot);
	CameraComponent->SetRelativeLocation(offset);
	CameraComponent->FieldOfView = captureConfig->data.fov;
}
void APanoCapture::RunRender(){
	moviePipelineSubsystem = GEngine->GetEngineSubsystem<UMoviePipelineQueueEngineSubsystem>();
	FScriptDelegate scriptDelegate = FScriptDelegate();
	scriptDelegate.BindUFunction(this,L"OnRenderFinish");
	moviePipelineSubsystem->OnRenderFinished.Add(scriptDelegate);
	if(captureConfig->video){
		UMoviePipelineExecutorJob* job = moviePipelineSubsystem->AllocateJob(levelSequenceVideo);
		UMoviePipelineOutputSetting* outputsetting;

		if(captureConfig->pathtracer)
			outputsetting = Cast<UMoviePipelineOutputSetting>(moviePipelineMasterConfigVideoPathTracer->FindOrAddSettingByClass(UMoviePipelineOutputSetting::StaticClass()));
		else
			outputsetting = Cast<UMoviePipelineOutputSetting>(moviePipelineMasterConfigVideo->FindOrAddSettingByClass(UMoviePipelineOutputSetting::StaticClass()));
		outputsetting->FileNameFormat = FString().Printf(L"%s.%d.{frame_number}",*campos,facenum%captureConfig->data.total);
		outputsetting->OutputResolution = FIntPoint(captureConfig->data.width,captureConfig->data.height);
		outputsetting->OutputDirectory.Path = FPaths::Combine(captureConfig->outputdir,L"MovieRenders");
		if(captureConfig->pathtracer)
			job->SetConfiguration(moviePipelineMasterConfigVideoPathTracer);
		else 
			job->SetConfiguration(moviePipelineMasterConfigVideo);
		
		moviePipelineSubsystem->RenderJob(job);
	}else{
		UMoviePipelineExecutorJob* job = moviePipelineSubsystem->AllocateJob(levelSequence);
		UMoviePipelineOutputSetting* outputsetting;
		
		if(captureConfig->pathtracer)
			outputsetting = Cast<UMoviePipelineOutputSetting>(moviePipelineMasterConfigPathTracer->FindOrAddSettingByClass(UMoviePipelineOutputSetting::StaticClass()));
		else
			outputsetting = Cast<UMoviePipelineOutputSetting>(moviePipelineMasterConfig->FindOrAddSettingByClass(UMoviePipelineOutputSetting::StaticClass()));
		outputsetting->FileNameFormat = FString().Printf(L"%s.%d.{frame_number}",*campos,facenum%captureConfig->data.total);
		outputsetting->OutputResolution = FIntPoint(captureConfig->data.width,captureConfig->data.height);
		outputsetting->OutputDirectory.Path = FPaths::Combine(captureConfig->outputdir,L"MovieRenders");
		if(captureConfig->pathtracer)
			job->SetConfiguration(moviePipelineMasterConfigPathTracer);
		else 
			job->SetConfiguration(moviePipelineMasterConfig);
		moviePipelineSubsystem->RenderJob(job);
	}
	
	
}
TArray<FString> APanoCapture::GetLayerObj(){
	TArray<FString> ls = TArray<FString>();
	// UMoviePipelineOutputSetting* outputsetting = Cast<UMoviePipelineOutputSetting>(moviePipelineMasterConfigVideo->FindOrAddSettingByClass(UMoviePipelineOutputSetting::StaticClass()));
	UMoviePipelineDeferredPassBase * renderPassSetting = Cast<UMoviePipelineDeferredPassBase >(moviePipelineMasterConfigVideo->FindOrAddSettingByClass(UMoviePipelineDeferredPassBase ::StaticClass()));
	
	for(FActorLayer l:renderPassSetting->ActorLayers){
		ls.Add(L".FinalImage" + l.Name.ToString());
	}
	if(!renderPassSetting->ActorLayers.Num())
		ls.Add(L"");
	
	if(renderPassSetting->bAddDefaultLayer){
		ls.Add(L".FinalImageDefaultLayer");
	}
	if(renderPassSetting->bRenderMainPass){
		ls.Add(L".FinalImage");
	}
	return ls;
}
void APanoCapture::Test(){
	UMoviePipelineOutputSetting* outputsetting = Cast<UMoviePipelineOutputSetting>(moviePipelineMasterConfigVideo->FindOrAddSettingByClass(UMoviePipelineOutputSetting::StaticClass()));
	UE_LOG(LogTemp, Log,TEXT("%s"),*outputsetting->OutputDirectory.Path);
	UMoviePipelineDeferredPassBase * renderPassSetting = Cast<UMoviePipelineDeferredPassBase >(moviePipelineMasterConfigVideo->FindOrAddSettingByClass(UMoviePipelineDeferredPassBase ::StaticClass()));
	UE_LOG(LogTemp, Log,TEXT("%s"),*renderPassSetting->GetName());
	for(FActorLayer l:renderPassSetting->ActorLayers){
		UE_LOG(LogTemp, Log,TEXT("%s"),*l.Name.ToString());
	}
	
}
void APanoCapture::OnRenderFinish(FMoviePipelineOutputData Results){
	// UE_LOG(LogTemp, Log, TEXT("=====================>quit"));
	UKismetSystemLibrary::QuitGame(GetWorld(),nullptr,EQuitPreference::Quit,false);
}