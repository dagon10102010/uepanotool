// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Camera/CameraActor.h"
#include "PanoCapture.generated.h"
class UCameraComponent;
struct FPanoCaptureConfig;
class UMoviePipelineQueueEngineSubsystem;
class ULevelSequence;

class UMoviePipelinePrimaryConfig;


UCLASS()
class UEPANOTOOL_API APanoCapture : public ACameraActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APanoCapture();
	// Called every frame
	

	// UPROPERTY(EditAnywhere)
	// ULevelSequence* customSequences;
	
	TArray<FString> GetLayerObj();
protected:
    virtual void BeginPlay() override;
	// virtual void Tick( float DeltaSeconds ) override;
	void SetupCamera(int face);
	void RunRender();
	UFUNCTION(CallInEditor, Category = "PanoCapture", meta = (AllowPrivateAccess = "true"))
	void Test();
	UFUNCTION()
	void OnRenderFinish(FMoviePipelineOutputData Results);


	UPROPERTY(EditAnywhere, Category = "PanoCapture", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<ULevelSequence> captureSequence;

private:
	
    // class USceneComponent* SceneComponent;
	// UPROPERTY(VisibleDefaultsOnly)
	class UCameraComponent* CameraComponent;

	FPanoCaptureConfig* captureConfig = nullptr;
	UMoviePipelineQueueEngineSubsystem* moviePipelineSubsystem = nullptr;
	ULevelSequence * levelSequence = nullptr;
	ULevelSequence * levelSequenceVideo = nullptr;


	UMoviePipelinePrimaryConfig * moviePipelineMasterConfigVideo = nullptr;
	UMoviePipelinePrimaryConfig * moviePipelineMasterConfig = nullptr;
	UMoviePipelinePrimaryConfig * moviePipelineMasterConfigVideoPathTracer = nullptr;
	UMoviePipelinePrimaryConfig * moviePipelineMasterConfigPathTracer = nullptr;
	
	int renderindex=0;
	int facenum = 0;
	FString campos=L"center";
};
