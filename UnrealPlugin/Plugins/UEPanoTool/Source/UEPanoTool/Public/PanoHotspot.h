// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PanoHotspot.generated.h"
class UArrowComponent;
class UBillboardComponent;
UCLASS()
class UEPANOTOOL_API APanoHotspot : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APanoHotspot();
	// Called every frame
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool CaptureRotation = false;
	// UPROPERTY(EditAnywhere, BlueprintReadWrite)
	// 	FString panoguid = TEXT("");
	// UPROPERTY(EditAnywhere, BlueprintReadWrite)
	// 	FString panoname = TEXT("");
	// UPROPERTY(EditAnywhere, BlueprintReadWrite)
	// 	FString profile = TEXT("");
	// UPROPERTY(EditAnywhere, BlueprintReadWrite)
	// 	FString cate = TEXT("");
	// // UPROPERTY(EditAnywhere, BlueprintReadWrite)
	// // 	float camheight = 1.5f;
	// UPROPERTY(EditAnywhere, BlueprintReadWrite)
	// 	TArray<FString>links;
	// UFUNCTION()	
	// void UpdateLink(TArray<AStreetViewPoint*> ls);
	// UFUNCTION(BlueprintCallable, CallInEditor)
	// void RenewGUID();

	// UFUNCTION(BlueprintCallable)
	// void LinkTo(TArray<AActor*> ls);

	// TArray<AStreetViewPoint*> linksactor;
protected:

	
	// FColor linkdebugcolor = FColor(0,255,0);
	// // Called when the game starts or when spawned
	// virtual bool ShouldTickIfViewportsOnly() const override {
	// 	return true;
	// }
	// virtual void Tick( float DeltaSeconds ) override;
private:
	UPROPERTY()
	class USceneComponent* SceneComponent;

	

#if WITH_EDITORONLY_DATA
	/** Component shown in the editor only to indicate character facing */
	UPROPERTY()
	UArrowComponent* ArrowComponent;
	UPROPERTY()
	UBillboardComponent* BillboardComponent;
	
#endif
	
};
