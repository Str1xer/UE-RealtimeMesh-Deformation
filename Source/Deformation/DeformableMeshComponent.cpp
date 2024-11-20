#include "DeformableMeshComponent.h"
#include "CollisionNodeComponent.h"

UDeformableMeshComponent::UDeformableMeshComponent() {
	RealtimeMeshComponent = CreateDefaultSubobject<URealtimeMeshComponent>(TEXT("Mesh"));

	PrimaryComponentTick.TickGroup = TG_PrePhysics;
	PrimaryComponentTick.bStartWithTickEnabled = true;
	PrimaryComponentTick.bCanEverTick = true;
	RegisterAllComponentTickFunctions(true);
	SetComponentTickEnabled(true);

	this->bHiddenInGame = false;
	this->SetCollisionProfileName(TEXT("BlockAll"));
	this->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
}

void UDeformableMeshComponent::BeginPlay()
{
	Super::BeginPlay();

	this->SetSphereRadius(10.f);

	RealtimeMeshComponent->AttachToComponent(this, FAttachmentTransformRules::KeepWorldTransform);
	RealtimeMeshComponent->SetRelativeLocation(FVector(0.f, 0.f, 0.f));

	PreviousComponentLocation = RealtimeMeshComponent->GetComponentLocation();

	this->SetSimulatePhysics(true);

	InitMesh();
	SpawnCollisionNodesOnMesh();
	SpreadVerticesByNodes();
}

void UDeformableMeshComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UDeformableMeshComponent::InitMesh() {
	// Read static mesh information. Required enabled CPU Only setting in mesh.
	UKismetProceduralMeshLibrary::GetSectionFromStaticMesh(StaticMesh, 0, 0, Vertices, Triangles, Normals, UVs, Tangents);

	int32 VerticesCount = Vertices.Num();
	int32 TrianglesCount = Triangles.Num();

	RealtimeMesh = RealtimeMeshComponent->InitializeRealtimeMesh<URealtimeMeshSimple>();

	RealtimeMesh->SetupMaterialSlot(0, "PrimaryMaterial", MaterialAsset);

	FRealtimeMeshStreamSet StreamSet;
	TRealtimeMeshBuilderLocal<uint16, FPackedNormal, FVector2DHalf, 1> Builder(StreamSet);

	Builder.EnableTangents();
	Builder.EnableTexCoords();
	Builder.EnableColors();
	Builder.EnablePolyGroups();

	for (int i = 0; i < VerticesCount; i++) {
		Builder.AddVertex(FVector3f(Vertices[i].X, Vertices[i].Y, Vertices[i].Z))
			.SetNormalAndTangent(FVector3f(Normals[i].X, Normals[i].Y, Normals[i].Z), FVector3f(Tangents[i].TangentX.X, Tangents[i].TangentX.Y, Tangents[i].TangentX.Z))
			.SetColor(FColor::White)
			.SetTexCoord(FVector2f(UVs[i].X, UVs[i].Y));
	}
	
	for (int i = 0; i < TrianglesCount; i+=3) {
		Builder.AddTriangle(Triangles[i], Triangles[i + 1], Triangles[i + 2]);
	}

	FRealtimeMeshLODKey LODKey = FRealtimeMeshLODKey(0);
	GroupKey = URealtimeMeshBlueprintFunctionLibrary::MakeSectionGroupKeyUnique(LODKey);

	RealtimeMesh->CreateSectionGroup(GroupKey, StreamSet);
}

void UDeformableMeshComponent::UpdateMesh() {
	RealtimeMesh->EditMeshInPlace(GroupKey, [this](FRealtimeMeshStreamSet& StreamSet)
		{
			TRealtimeMeshBuilderLocal<uint16, FPackedNormal, FVector2DHalf, 1> Builder(StreamSet);

			for (int NodeIdx = 0; NodeIdx < CollisionNodes.Num(); NodeIdx++) {
				auto NodeVertices = *CollisionNodes[NodeIdx]->Vertices;
				auto NodePosition = CollisionNodes[NodeIdx]->Location;

				for (int VertexIdx = 0; VertexIdx < CollisionNodes[NodeIdx]->Vertices->Num(); VertexIdx++) {
					Builder.EditVertex(NodeVertices[VertexIdx]).SetPosition(FVector3f(NodePosition.X, NodePosition.Y, NodePosition.Z));
				}
			}

			TSet<FRealtimeMeshStreamKey> Res;
			Res.Add(FRealtimeMeshStreams::Position);
			
			return Res;
		}
	);
}

void UDeformableMeshComponent::SpawnCollisionNode(FVector Location) {
	UCollisionNodeComponent* CreatedCollisionNode = NewObject<UCollisionNodeComponent>(RealtimeMesh, TEXT("Collision Node " + CollisionNodes.Num()));
	CreatedCollisionNode->DeformableMesh = this;
	CreatedCollisionNode->AttachToComponent(this, FAttachmentTransformRules::KeepWorldTransform);
	CreatedCollisionNode->RegisterComponent();
	CreatedCollisionNode->SetRelativeLocation(Location);
	CreatedCollisionNode->Location = Location;
	//CreatedCollisionNode->SetSimulatePhysics(true);
	CollisionNodesCount++;
	CollisionNodes.Push(CreatedCollisionNode);
}

void UDeformableMeshComponent::SpawnCollisionNodesOnMesh() {
	CollisionNodes = TArray<UCollisionNodeComponent*>();
	FVector ComponentLocation = this->GetRelativeLocation();
	TSet<FVector> SetOfVertices = TSet<FVector>(Vertices);
	for (FVector& Element : SetOfVertices) {
		SpawnCollisionNode(Element);
	}
}

void UDeformableMeshComponent::SpreadVerticesByNodes() {
	for (UCollisionNodeComponent* CollisionNode : CollisionNodes) {
		for (int VertexId = 0; VertexId < Vertices.Num(); VertexId++) {
			if ((CollisionNode->Location - Vertices[VertexId]).SizeSquared() < 0.1f) {
				CollisionNode->Vertices->Push(VertexId);
				//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::Printf(TEXT("%d"), CollisionNode->Vertices->Num()));
			}
		}
	}
}

void UDeformableMeshComponent::MoveNodes(FVector NormalImpulse, const FHitResult& Hit) {
	float SharedMass = 0.0f;
	auto EngineWorld = GetWorld();
	
	AActor* SelfActor = this->GetOwner()->GetRootComponent()->GetOwner();

	for (UCollisionNodeComponent* CollisionNode : CollisionNodes) {
		FHitResult TraceHit = CollisionNode->LineTrace(EngineWorld, SelfActor, CollisionNode->GetComponentLocation() + Hit.ImpactNormal * -10, CollisionNode->GetComponentLocation() + Hit.ImpactNormal * -20);
		if (TraceHit.bBlockingHit) {
			FVector Impulse = NormalImpulse / 100 * 0.01 * ((10 - Hit.Distance) / 10);
			if (Impulse.Length() < 10) continue;
			CollisionNode->AddWorldOffset(Impulse);
			CollisionNode->Location += Impulse;
		}
	}

	UpdateMesh();
}