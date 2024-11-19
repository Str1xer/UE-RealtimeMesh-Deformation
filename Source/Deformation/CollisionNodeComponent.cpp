#include "CollisionNodeComponent.h"

UCollisionNodeComponent::UCollisionNodeComponent() {
	this->bHiddenInGame = false;
	this->SetNotifyRigidBodyCollision(true);
	this->SetGenerateOverlapEvents(true);
	//this->SetEnableGravity(false);
	this->SetCollisionProfileName(TEXT("DeformationNode"));
	//this->SetConstraintMode(EDOFMode::SixDOF);
	//this->BodyInstance.bLockXRotation = true;
	//this->BodyInstance.bLockYRotation = true;
	//this->BodyInstance.bLockZRotation = true;
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
	FVector Impulse = NormalImpulse / 100 * 0.01;
	if (Impulse.Length() < 0.1) return;
	GEngine->AddOnScreenDebugMessage(-1, 60.f, FColor::White, FString::Printf(TEXT("I Hit: %s %s %s %s"), 
		*OtherActor->GetName(), 
		*Hit.ImpactNormal.ToString(),
		*NormalImpulse.ToString(),
		*FString::SanitizeFloat(OtherActor->GetVelocity().Length())
	));
	GEngine->AddOnScreenDebugMessage(-1, 60.f, FColor::Yellow, FString::Printf(TEXT("Impulse: %s"),
		*Impulse.ToString()
	));
	HitComp->AddWorldOffset(Impulse);
	Location = Location + Impulse;
}