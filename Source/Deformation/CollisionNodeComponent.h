#pragma once

#include "CoreMinimal.h"
#include "Components/SphereComponent.h"
#include "CollisionNodeComponent.generated.h"

class UDeformableMeshComponent;

UCLASS()
class DEFORMATION_API UCollisionNodeComponent : public USphereComponent
{
	GENERATED_BODY()
	
	UCollisionNodeComponent();

protected:
	virtual void BeginPlay() override;
	
	UFUNCTION()
	void OnNodeHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);
	
public:
	FHitResult LineTrace(UWorld* EngineWorld, AActor* Owner, FVector StartLocation, FVector EndLocation, bool bIsDebug);

	int NodeId;
	UDeformableMeshComponent* DeformableMesh;
	FVector Location;
	TArray<int32>* Vertices = new TArray<int32>();
};
