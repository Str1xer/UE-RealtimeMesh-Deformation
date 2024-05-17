#include "DeformableMeshComponent.h"

UDeformableMeshComponent::UDeformableMeshComponent() {
	RealtimeMeshComponent = CreateDefaultSubobject<URealtimeMeshComponent>(TEXT("Mesh"));
}

void UDeformableMeshComponent::BeginPlay()
{
	Super::BeginPlay();

	
	RealtimeMeshComponent->AttachToComponent(this, FAttachmentTransformRules::KeepWorldTransform);
	RealtimeMeshComponent->SetRelativeLocation(FVector(0.f, 0.f, 0.f));

	BuildMesh();
}

void UDeformableMeshComponent::BuildMesh() {
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