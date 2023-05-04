// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PanoCapture.generated.h"
class UCameraComponent;
struct FPanoCaptureConfig;
class UMoviePipelineQueueEngineSubsystem;
class ULevelSequence;
class UMoviePipelineMasterConfig;

UCLASS()
class UEPANOTOOL_API APanoCapture : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APanoCapture();
	// Called every frame
	
protected:
    virtual void BeginPlay() override;
	// virtual void Tick( float DeltaSeconds ) override;
	void SetupCamera(int face);
	UFUNCTION(CallInEditor)
	void RunRender();
	UFUNCTION()
	void OnRenderFinish(FMoviePipelineOutputData Results);

private:
	UPROPERTY()
    class USceneComponent* SceneComponent;
	UPROPERTY(VisibleDefaultsOnly)
	class UCameraComponent* CameraComponent;

	FPanoCaptureConfig* captureConfig = nullptr;
	UMoviePipelineQueueEngineSubsystem* moviePipelineSubsystem = nullptr;
	ULevelSequence * levelSequence = nullptr;
	ULevelSequence * levelSequenceVideo = nullptr;
	UMoviePipelineMasterConfig * moviePipelineMasterConfigVideo = nullptr;
	UMoviePipelineMasterConfig * moviePipelineMasterConfig = nullptr;
	int renderindex=0;
	int facenum = 0;
	FString campos=L"center";
};
