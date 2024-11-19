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
	FRealtimeMeshSectionGroupKey GroupKey = URealtimeMeshBlueprintFunctionLibrary::MakeSectionGroupKeyUnique(LODKey);

	RealtimeMesh->CreateSectionGroup(GroupKey, StreamSet);
}

void UDeformableMeshComponent::SpawnCollisionNode(FVector Location) {
	UCollisionNodeComponent* CreatedCollisionNode = NewObject<UCollisionNodeComponent>(RealtimeMesh, TEXT("Collision Node " + CollisionNodes.Num()));
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
			if ((CollisionNode->Location - Vertices[VertexId]).SizeSquared() < 1.0f) {
				CollisionNode->Vertices->Push(VertexId);
				//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::Printf(TEXT("%d"), CollisionNode->Vertices->Num()));
			}
		}
	}
}