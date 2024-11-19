#include "CollisionNodeComponent.h"

UCollisionNodeComponent::UCollisionNodeComponent() {
	this->bHiddenInGame = false;
	this->SetNotifyRigidBodyCollision(true);
	this->SetGenerateOverlapEvents(true);
	this->SetEnableGravity(false);
	this->SetCollisionProfileName(TEXT("DeformationNode"));
	this->SetConstraintMode(EDOFMode::SixDOF);
	this->BodyInstance.bLockXRotation = true;
	this->BodyInstance.bLockYRotation = true;
	this->BodyInstance.bLockZRotation = true;
	this->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
}

void UCollisionNodeComponent::BeginPlay() {
	Super::BeginPlay();

	this->SetSphereRadius(10.f);
	this->SetLinearDamping(1.0f);
	this->SetMassOverrideInKg("", 25.0f);
	
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("Node Begin Play")));

	this->OnComponentHit.AddDynamic(this, &UCollisionNodeComponent::OnNodeHit);
}

void UCollisionNodeComponent::OnNodeHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit) {
	/*GEngine->AddOnScreenDebugMessage(-1, 60.f, FColor::White, FString::Printf(TEXT("I Hit: %s %s %s %s"), 
		*OtherActor->GetName(), 
		*Hit.ImpactNormal.ToString(),
		*OtherActor->GetVelocity().ToString(),
		*FString::SanitizeFloat(OtherActor->GetVelocity().Length())
	));*/
}