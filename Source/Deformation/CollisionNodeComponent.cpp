#include "CollisionNodeComponent.h"
#include "DeformableMeshComponent.h"
// Draw Debug
#include "Kismet/KismetSystemLibrary.h"

UCollisionNodeComponent::UCollisionNodeComponent() 
{
	this->SetNotifyRigidBodyCollision(true);
	this->SetGenerateOverlapEvents(true);
	this->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	this->SetLinearDamping(10.f);
	this->SetAngularDamping(100.f);
	this->SetCollisionProfileName(TEXT("DeformationNode"));
}

void UCollisionNodeComponent::BeginPlay() 
{
	Super::BeginPlay();

	this->SetMassOverrideInKg("", 25.0f);
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