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
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable)
	void InitMesh();
	
	UDeformableMeshComponent();

	void SpawnCollisionNode(FVector location);
	void SpawnCollisionNodesOnMesh();
	void SpreadVerticesByNodes();
	void FollowNodeForRoot();
	
	int CollisionNodesCount = 0;
	TArray<UCollisionNodeComponent*> CollisionNodes;

	TArray<FVector> Vertices;
	TArray<int32> Triangles;
	TArray<FVector> Normals;
	TArray<FVector2D> UVs;
	TArray<FProcMeshTangent> Tangents;

	FVector PreviousComponentLocation;
	
public: 
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Mesh")
	UStaticMesh* StaticMesh;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Mesh")
	UMaterial* MaterialAsset;
};
