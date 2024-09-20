#include "CollisionNodeComponent.h"

UCollisionNodeComponent::UCollisionNodeComponent() {
	this->bHiddenInGame = false;
	this->SetCollisionProfileName("BlockAll");
	this->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	
}

void UCollisionNodeComponent::BeginPlay() {
	Super::BeginPlay();

	this->SetSphereRadius(10.f);
	this->SetMassOverrideInKg("", 100.0f);
}