// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RealtimeMeshComponent.h"
#include "KismetProceduralMeshLibrary.h"
#include "ProceduralMeshComponent.h"
#include "RealtimeMeshLibrary.h"
#include "RealtimeMeshSimple.h"
#include "Components/SphereComponent.h"
#include "PhysicsEngine/PhysicsConstraintComponent.h"
#include "Mesh/RealtimeMeshBlueprintMeshBuilder.h"
#include "DeformableMeshComponent.generated.h"

class UCollisionNodeComponent;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class DEFORMATION_API UDeformableMeshComponent : public USphereComponent
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

	void SpawnCollisionNode(FVector Location);
	void SpawnCollisionNodeWithPhysicsConstraint(FVector Location);
	void GenerateCollisionModel();
	void ComputeWeights();
	TArray<float> ComputeWeightsForVertex(FVector VertexLocation);

	int CollisionNodesCount = 0;
	
	// Mesh Data
	TArray<FVector> Vertices;
	TArray<int32> Triangles;
	TArray<FVector> Normals;
	TArray<FVector2D> UVs;
	TArray<FProcMeshTangent> Tangents;

	// Cage-Based Deformation Weights
	TArray<TArray<float>> DeformationTransferWeights;

	// Cage Data
	TArray<FVector> CageVertices = TArray<FVector>();
	TArray<int32> CageTriangles = TArray<int32>();

	FRealtimeMeshSectionGroupKey GroupKey;

	FVector PreviousComponentLocation;
	
public: 
	const float Epsilon = 1e-4f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Mesh")
	UStaticMesh* CageMesh;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Mesh")
	float CageNodeRadius = 10.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Mesh")
	bool bUsePhysicsConstraint = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Mesh")
	UStaticMesh* StaticMesh;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Mesh")
	UMaterial* MaterialAsset;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Development")
	bool bIsDebug = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Debug")
	TArray<UCollisionNodeComponent*> CollisionNodes;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Debug")
	TArray<UPhysicsConstraintComponent*> NodeConstraints;

	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Debug")
	void UpdateMesh();

	UFUNCTION(BlueprintCallable)
	void MoveNodes(int NodeId, FVector NormalImpulse, const FHitResult& Hit);
};
