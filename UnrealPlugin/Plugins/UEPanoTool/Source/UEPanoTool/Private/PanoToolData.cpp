#include "PanoToolData.h"
#include "JsonObjectConverter.h"
#include "Misc/FileHelper.h"
#include "PanoHotspot.h"
#include "Kismet/GameplayStatics.h"


FString FPanoCaptureStyle::GetEyeName(int slice){
    if(!stereo){
        return L"center";
    }else if(slice< total){
        return L"left";
    }else{
        return L"right";
    }
}
FVector FPanoCaptureStyle::GetPos(int slice){    
    if(!stereo){
        return FVector(0);
    }
    FQuat q = FQuat().MakeFromEuler(GetRotate(slice));
    if(slice< total){
        return q.RotateVector(FVector(0,3.2,0));
    }else{
        return q.RotateVector(FVector(0,-3.2,0));
    }
}
FVector FPanoCaptureStyle::GetRotate(int slice){
    int rslice = slice%total;
    int count = 0;
    for (int j = 0; j < horizontalStep.Num(); j++)
    {
        int step = horizontalStep[j];
        for (int i = 0; i < step; i++)
        {
            if(rslice==count){
                float yaw = 360.0f/step*i;
                float pitch = horizontalPitch[j];
                return FVector(0,pitch,yaw);
            }
            count++;
        }
    }
    return FVector(0);
}
void FPanoCaptureConfig::Save(FString filepath){
    FString s;
    FString fPath = FPaths::Combine(FPaths::ProjectPluginsDir(),L"UEPanoTool/Tool/capture.config");
    if(!filepath.IsEmpty()){
        fPath = filepath;
    }
	FJsonObjectConverter::UStructToJsonObjectString(FPanoCaptureConfig::StaticStruct(),this, s);
	FFileHelper::SaveStringToFile(s, *fPath);
}

void FPanoCaptureConfig::Load(FString filepath){
    FString s;
	FString fPath = FPaths::Combine(FPaths::ProjectPluginsDir(),L"UEPanoTool/Tool/capture.config");
    if(!filepath.IsEmpty()){
        fPath = filepath;
    }
    if (!FFileHelper::LoadFileToString(s, *fPath))
		return;
    FJsonObjectConverter::JsonObjectStringToUStruct(s,this);
}

FPanoPoint::FPanoPoint(APanoHotspot* hp){
    pos = hp->GetActorLocation();
    capturerot = hp->CaptureRotation;
    rot = hp->GetActorQuat().Euler();
    name = hp->GetActorNameOrLabel();

}

void FPanoPointList::Scan(UWorld*World){
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(World, APanoHotspot::StaticClass(), FoundActors);
    data.Empty();
    for (AActor* a : FoundActors){
        APanoHotspot* hp = Cast<APanoHotspot>(a);
        data.Add(FPanoPoint(hp));
    }

}
void FPanoPointList::Save(FString filepath){
    FString s;
    FString fPath = FPaths::Combine(FPaths::ProjectPluginsDir(),L"UEPanoTool/Tool/list.json");
    if(!filepath.IsEmpty()){
        fPath = filepath;
    }
	FJsonObjectConverter::UStructToJsonObjectString(FPanoPointList::StaticStruct(),this, s);
	FFileHelper::SaveStringToFile(s, *fPath);
}
void FPanoPointList::Load(FString filepath){
    FString s;
	FString fPath = FPaths::Combine(FPaths::ProjectPluginsDir(),L"UEPanoTool/Tool/list.json");
    if(!filepath.IsEmpty()){
        fPath = filepath;
    }
    if (!FFileHelper::LoadFileToString(s, *fPath))
		return;
    FJsonObjectConverter::JsonObjectStringToUStruct(s,this);
}