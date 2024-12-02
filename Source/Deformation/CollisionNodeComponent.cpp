#include "CollisionNodeComponent.h"
#include "DeformableMeshComponent.h"
// Draw Debug
#include "Kismet/KismetSystemLibrary.h"

UCollisionNodeComponent::UCollisionNodeComponent() {
	this->bHiddenInGame = false;
	this->SetNotifyRigidBodyCollision(true);
	this->SetGenerateOverlapEvents(true);
	this->SetCollisionProfileName(TEXT("DeformationNode"));
	this->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
}

void UCollisionNodeComponent::BeginPlay() {
	Super::BeginPlay();

	this->SetMassOverrideInKg("", 25.0f);
	this->OnComponentHit.AddDynamic(this, &UCollisionNodeComponent::OnNodeHit);
}

void UCollisionNodeComponent::OnNodeHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit) {
	if (!DeformableMesh) return;

	DeformableMesh->MoveNodes(NormalImpulse, Hit);
}

FHitResult UCollisionNodeComponent::LineTrace(UWorld* EngineWorld, AActor* Owner, FVector StartLocation, FVector EndLocation, bool bIsDebug) {
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

	if (!bIsDebug) return HitResult;

	TArray<AActor*>* IgnoreActorsArray = new TArray<AActor*>();
	IgnoreActorsArray->Add(Owner);

	UKismetSystemLibrary::LineTraceSingle(
		EngineWorld,
		StartLocation,
		EndLocation,
		UEngineTypes::ConvertToTraceType(ECollisionChannel::ECC_GameTraceChannel2),
		false,
		*IgnoreActorsArray,
		EDrawDebugTrace::ForDuration,
		HitResult, true,
		FLinearColor::Red,
		FLinearColor::Green, 10.0f
	);

	return HitResult;
}