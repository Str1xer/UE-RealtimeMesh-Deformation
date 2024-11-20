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

	this->SetSphereRadius(10.f);
	this->SetLinearDamping(0.01f);
	this->SetMassOverrideInKg("", 25.0f);
	
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("Node Begin Play")));

	this->OnComponentHit.AddDynamic(this, &UCollisionNodeComponent::OnNodeHit);
}

void UCollisionNodeComponent::OnNodeHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit) {
	if (!DeformableMesh) return;

	//FVector Impulse = NormalImpulse / 100 * 0.01;
	//if (Impulse.Length() < 2) return;
	
	//GEngine->AddOnScreenDebugMessage(-1, 60.f, FColor::White, FString::Printf(TEXT("I Hit: %s %s %s %s"), 
	//	*OtherActor->GetName(), 
	//	*Hit.ImpactNormal.ToString(),
	//	*NormalImpulse.ToString(),
	//	*FString::SanitizeFloat(OtherActor->GetVelocity().Length())
	//));
	//GEngine->AddOnScreenDebugMessage(-1, 60.f, FColor::Yellow, FString::Printf(TEXT("Impulse: %s"),
	//	*Impulse.ToString()
	//));
	//LineTrace(GetWorld(), GetOwner(), HitComp->GetComponentLocation(), HitComp->GetComponentLocation() + Hit.ImpactNormal * -100);

	//HitComp->AddWorldOffset(Impulse);
	//Location = Location + Impulse;
	DeformableMesh->MoveNodes(NormalImpulse, Hit);
	//DeformableMesh->UpdateMesh();
}

FHitResult UCollisionNodeComponent::LineTrace(UWorld* EngineWorld, AActor* Owner, FVector StartLocation, FVector EndLocation) {
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