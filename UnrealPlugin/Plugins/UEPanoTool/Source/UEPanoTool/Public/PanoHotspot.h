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
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bSkip = false;
	
protected:

	
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
