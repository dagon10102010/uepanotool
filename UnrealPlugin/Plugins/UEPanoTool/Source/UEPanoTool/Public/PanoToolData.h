#pragma once

#include "CoreMinimal.h"
#include "PanoToolData.generated.h"
class APanoHotspot;
USTRUCT()
struct UEPANOTOOL_API FPanoCaptureStyle {
	GENERATED_BODY()

	UPROPERTY()
	FString name;

	UPROPERTY()
	bool stereo = false;

	UPROPERTY()
	float fov = 90;

	UPROPERTY()
	TArray<int> horizontalStep;

    UPROPERTY()
	TArray<float> horizontalPitch;

    UPROPERTY()
	int basewidth=20;

    UPROPERTY()
	int baseheight=20;

    UPROPERTY()
	int width=512;

    UPROPERTY()
	int height=512;

    UPROPERTY()
	int outwidth=2048;

    UPROPERTY()
	int outheight=1024;

    UPROPERTY()
	int total=6;

	FString GetEyeName(int slice);
	FVector GetPos(int slice);
	FVector GetRotate(int slice);
	
    
	FPanoCaptureStyle(){}

   
	
};

USTRUCT()
struct UEPANOTOOL_API FPanoCaptureStyleData {
	GENERATED_BODY()

	UPROPERTY()
	TArray<FPanoCaptureStyle> data;
};

USTRUCT()
struct UEPANOTOOL_API FPanoCaptureConfig {
	GENERATED_BODY()

	UPROPERTY()
	FString ued;

	UPROPERTY()
	FString project;

	UPROPERTY()
	FString map;

    UPROPERTY()
	bool video=true;

	UPROPERTY()
	int maxslice=30;

    UPROPERTY()
	int startframe=0;

	UPROPERTY()
	int endframe=0;

	UPROPERTY()
	int step=3;

    UPROPERTY()
	int projector=0;

	UPROPERTY()
	bool clearImg=false;

	UPROPERTY()
	bool jpg=false;

	UPROPERTY()
	bool png=false;

	UPROPERTY()
	bool tiff=false;

    UPROPERTY()
    FPanoCaptureStyle data = FPanoCaptureStyle();

	FPanoCaptureConfig(){}

    void Save(FString filepath=L"");
    void Load(FString filepath=L"");
	bool IsFrontFace(int slice);
   
};

USTRUCT()
struct UEPANOTOOL_API FPanoPoint {
	GENERATED_BODY()

	UPROPERTY()
	FString name;

	UPROPERTY()
	FVector pos;

    UPROPERTY()
	FVector rot;

	UPROPERTY()
	bool capturerot;
    
	FPanoPoint(){}
	FPanoPoint(APanoHotspot* hp);

   
};

USTRUCT()
struct UEPANOTOOL_API FPanoPointList {
	GENERATED_BODY()

	UPROPERTY()
	TArray<FPanoPoint> data;
    
	FPanoPointList(){}

	void Scan(UWorld*World);
    void Save(FString filepath=L"");
    void Load(FString filepath=L"");
   
};

