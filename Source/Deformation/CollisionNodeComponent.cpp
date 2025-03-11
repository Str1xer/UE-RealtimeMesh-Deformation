#include "CollisionNodeComponent.h"
#include "DeformableMeshComponent.h"
// Draw Debug
#include "Kismet/KismetSystemLibrary.h"

UCollisionNodeComponent::UCollisionNodeComponent() 
{
	this->bHiddenInGame = true;
	this->SetNotifyRigidBodyCollision(true);
	this->SetGenerateOverlapEvents(true);
	this->SetCollisionProfileName(TEXT("DeformationNode"));
	this->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
}

void UCollisionNodeComponent::BeginPlay() 
{
	Super::BeginPlay();

	this->OnComponentHit.AddDynamic(this, &UCollisionNodeComponent::OnNodeHit);
}

void UCollisionNodeComponent::OnNodeHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit) 
{
	if (!DeformableMesh) return;

	DeformableMesh->MoveNodes(NodeId, NormalImpulse, Hit);
}

FHitResult UCollisionNodeComponent::LineTrace(UWorld* EngineWorld, AActor* Owner, FVector StartLocation, FVector EndLocation, bool bIsDebug) 
{
	FHitResult HitResult;
	FCollisionQueryParams CollisionParams;
	CollisionParams.AddIgnoredActor(Owner);

	EngineWorld->SweepSingleByChannel(
		HitResult,
		StartLocation,
		EndLocation,
		FQuat(0, 0, 0, 0),
		ECollisionChannel::ECC_GameTraceChannel2,
		FCollisionShape::MakeSphere(16),
		CollisionParams
	);

	

	return HitResult;
}