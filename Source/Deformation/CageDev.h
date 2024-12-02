// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "CageDev.generated.h"

/**
 * 
 */
UCLASS()
class DEFORMATION_API UCageDev : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	UFUNCTION(BlueprintCallable)
	static TArray<float> ComputeWeights( 
		FVector Point,
		TArray<FVector> CageVertices,
		TArray<int32> CageFaces
	);
};
