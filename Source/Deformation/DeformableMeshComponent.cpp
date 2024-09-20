#include "DeformableMeshComponent.h"
#include "CollisionNodeComponent.h"

UDeformableMeshComponent::UDeformableMeshComponent() {
	RealtimeMeshComponent = CreateDefaultSubobject<URealtimeMeshComponent>(TEXT("Mesh"));
}

void UDeformableMeshComponent::BeginPlay()
{
	Super::BeginPlay();

	RealtimeMeshComponent->AttachToComponent(this, FAttachmentTransformRules::KeepWorldTransform);
	RealtimeMeshComponent->SetRelativeLocation(FVector(0.f, 0.f, 0.f));

	InitMesh();
	SpawnCollisionNodesOnMesh();
	SpreadVerticesByNodes();
}

void UDeformableMeshComponent::InitMesh() {
	// Read static mesh information. Required enabled CPU Only setting in mesh.
	UKismetProceduralMeshLibrary::GetSectionFromStaticMesh(StaticMesh, 0, 0, Vertices, Triangles, Normals, UVs, Tangents);

	int32 VerticesCount = Vertices.Num();
	int32 TrianglesCount = Triangles.Num();

	RealtimeMesh = RealtimeMeshComponent->InitializeRealtimeMesh<URealtimeMeshSimple>();

	RealtimeMesh->SetupMaterialSlot(0, "PrimaryMaterial", MaterialAsset);

	URealtimeMeshStreamSet* StreamSet = URealtimeMeshStreamUtils::MakeRealtimeMeshStreamSet();
	FRealtimeMeshLocalBuilder Builder = URealtimeMeshStreamUtils::MakeLocalMeshBuilder(
		StreamSet, ERealtimeMeshSimpleStreamConfig::Normal, ERealtimeMeshSimpleStreamConfig::Normal, ERealtimeMeshSimpleStreamConfig::Normal,
		ERealtimeMeshSimpleStreamConfig::Normal, true, 1, false);

	for (int i = 0; i < VerticesCount; i++) {
		URealtimeMeshStreamUtils::AddVertex(Builder, Vertices[i], Tangents[i].TangentX, Normals[i], FLinearColor::White, UVs[i]);
	}
	
	for (int i = 0; i < TrianglesCount; i+=3) {
		int32 idx;
		URealtimeMeshStreamUtils::AddTriangle(Builder, idx, Triangles[i], Triangles[i + 1], Triangles[i + 2], 0);
	}

	FRealtimeMeshSimpleCompletionCallback Delegate;

	FRealtimeMeshLODKey LODKey = FRealtimeMeshLODKey(0);
	FRealtimeMeshSectionGroupKey GroupKey = URealtimeMeshBlueprintFunctionLibrary::MakeSectionGroupKeyUnique(LODKey);

	RealtimeMesh->CreateSectionGroup(GroupKey, StreamSet, Delegate);
}

void UDeformableMeshComponent::SpawnCollisionNode(FVector Location) {
	UCollisionNodeComponent* CreatedCollisionNode = NewObject<UCollisionNodeComponent>(this, TEXT("Collision Node " + CollisionNodes.Num()));
	CreatedCollisionNode->AttachToComponent(GetOwner()->GetDefaultAttachComponent(), FAttachmentTransformRules::KeepWorldTransform);
	CreatedCollisionNode->RegisterComponent();
	CreatedCollisionNode->SetRelativeLocation(Location);
	CreatedCollisionNode->Location = Location;
	CollisionNodes.Push(CreatedCollisionNode);
}

void UDeformableMeshComponent::SpawnCollisionNodesOnMesh() {
	CollisionNodes = TArray<UCollisionNodeComponent*>();
	FVector ComponentLocation = this->GetRelativeLocation();
	TSet<FVector> SetOfVertices = TSet<FVector>(Vertices);
	for (FVector& Element : SetOfVertices) {
		SpawnCollisionNode(ComponentLocation + Element);
	}
}

void UDeformableMeshComponent::SpreadVerticesByNodes() {
	for (UCollisionNodeComponent* CollisionNode : CollisionNodes) {
		for (int VertexId = 0; VertexId < Vertices.Num(); VertexId++) {
			if ((CollisionNode->Location - Vertices[VertexId]).SizeSquared() < 1.0f) {
				CollisionNode->Vertices->Push(VertexId);
				GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::Printf(TEXT("%d"), CollisionNode->Vertices->Num()));
			}
		}
	}
}