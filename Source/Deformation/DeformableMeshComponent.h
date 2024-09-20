// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RealtimeMeshComponent.h"
#include "KismetProceduralMeshLibrary.h"
#include "ProceduralMeshComponent.h"
#include "RealtimeMeshLibrary.h"
#include "RealtimeMeshSimple.h"
#include "Mesh/RealtimeMeshBlueprintMeshBuilder.h"
#include "DeformableMeshComponent.generated.h"

class UCollisionNodeComponent;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class DEFORMATION_API UDeformableMeshComponent : public URealtimeMeshComponent
{
	GENERATED_BODY()

private:
	URealtimeMeshComponent* RealtimeMeshComponent;
	URealtimeMeshSimple* RealtimeMesh;


protected:
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable)
	void InitMesh();
	void OnComplete();

	UDeformableMeshComponent();

	void SpawnCollisionNode(FVector location);
	void SpawnCollisionNodesOnMesh();
	void SpreadVerticesByNodes();
	TArray<UCollisionNodeComponent*> CollisionNodes;

	TArray<FVector> Vertices;
	TArray<int32> Triangles;
	TArray<FVector> Normals;
	TArray<FVector2D> UVs;
	TArray<FProcMeshTangent> Tangents;
	
public: 
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Mesh")
	UStaticMesh* StaticMesh;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Mesh")
	UMaterial* MaterialAsset;
};
