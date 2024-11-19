#pragma once

#include "CoreMinimal.h"
#include "Components/SphereComponent.h"
#include "CollisionNodeComponent.generated.h"


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
	FVector Location;
	TArray<int32>* Vertices = new TArray<int32>();
};
