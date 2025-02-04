// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "PanoToolData.h"
#include "PanoCameraComponent.generated.h"

class UCameraComponent;
class ULevelSequence;
class UMoviePipelineQueue;
// #if ENGINE_MINOR_VERSION == 1
//     class UMoviePipelineMasterConfig;
// #else //2
	class UMoviePipelinePrimaryConfig;
// #endif

UCLASS(HideCategories=(Mobility, Rendering, LOD), Blueprintable, ClassGroup=Camera, meta=(BlueprintSpawnableComponent))
class UEPANOTOOLEDITOR_API UPanoCameraComponent : public USceneComponent
{
	GENERATED_BODY()
public:
    UPanoCameraComponent();

    UPROPERTY(EditAnywhere)
    int renderIndex=0;
    UPROPERTY(EditAnywhere)
    TObjectPtr<ULevelSequence> sequence;
    UPROPERTY(EditAnywhere)
    TObjectPtr<UMoviePipelineQueue> mrq;
    UPROPERTY(EditAnywhere,meta = (ClampMin = "1024", ClampMax = "32768", UIMin = "1024", UIMax = "32768"))
    int outputWidth = 2048;
    UPROPERTY(EditAnywhere)
    TEnumAsByte<ECaptureStyle> captureStyle = ECaptureStyle::mono6;
    UPROPERTY(EditAnywhere)
    TEnumAsByte<EOutputStyle> outputStyle = EOutputStyle::EquirectAngular;
    UPROPERTY(EditAnywhere)
    FString outputDiretory = "";
    UPROPERTY(EditAnywhere)
    bool tiff;
    UPROPERTY(EditAnywhere)
    bool jpg = true;
    UPROPERTY(EditAnywhere)
    bool png;
    UPROPERTY(EditAnywhere)
    bool bPathTracer = false;

    UPROPERTY(EditAnywhere)
    bool bResumeRender = false;

    UPROPERTY(EditAnywhere)
    int ResumeRenderStart = 0;

    UPROPERTY(EditAnywhere)
    int ResumeRenderEnd = 0;

    UFUNCTION(CallInEditor)
    void CreateMRQ();

    void TestRenderCameraView();
protected:
    virtual void BeginPlay() override;

private:
    class UCameraComponent* CameraComponent = nullptr;
    struct FPanoCaptureConfig* captureConfig = nullptr;
    FString captureStyleDataText = L"";
    static void OnChangeRenderIndex(IConsoleVariable * val);
    void SetupConfig();
    void GenStitch();
    TArray<FString> GetLayerObj();
    FString BuildOutputString(int eye,FString layer);
    IConsoleVariable * pCvar = nullptr;
    ULevelSequence * zeroSequence = nullptr;
    TObjectPtr<UMoviePipelineQueue> mrqdefault;
// #if ENGINE_MINOR_VERSION == 1
//     UMoviePipelineMasterConfig * moviePipelineMasterConfigVideo = nullptr;
	
// 	UMoviePipelineMasterConfig * moviePipelineMasterConfigVideoPathTracer = nullptr;
// #else //2
	UMoviePipelinePrimaryConfig * moviePipelineMasterConfigVideo = nullptr;
	UMoviePipelinePrimaryConfig * moviePipelineMasterConfigVideoPathTracer = nullptr;
// #endif
	    
};
