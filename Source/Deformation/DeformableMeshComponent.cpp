#include "DeformableMeshComponent.h"
#include "CollisionNodeComponent.h"

UDeformableMeshComponent::UDeformableMeshComponent() 
{
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
	RealtimeMeshComponent->SetRelativeRotation(FQuat(0, 0, 0, 0));

	PreviousComponentLocation = RealtimeMeshComponent->GetComponentLocation();
	
	InitMesh();
	GenerateCollisionModel();
	ComputeWeights();
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

void UDeformableMeshComponent::UpdateMesh()
{	
	TArray<FVector> NewVertices;
	NewVertices.SetNum(Vertices.Num());

	FCriticalSection Mutex;
	ParallelFor(Vertices.Num(), [&](int32 VertexIdx)
		{
			FVector NewLocation = FVector(0, 0, 0);

			for (int32 NodeIdx = 0; NodeIdx < CollisionNodes.Num(); NodeIdx++) {
				//auto NodePosition = CollisionNodes[NodeIdx]->GetRelativeLocation();
				FVector NodePosition = this->GetComponentTransform().InverseTransformPosition(CollisionNodes[NodeIdx]->GetComponentLocation());

				NewLocation += DeformationTransferWeights[VertexIdx][NodeIdx] * NodePosition;
			}

			Mutex.Lock();
			NewVertices[VertexIdx] = NewLocation;
			Mutex.Unlock();
		}
	);

	RealtimeMesh->EditMeshInPlace(GroupKey, [this, &NewVertices](FRealtimeMeshStreamSet& StreamSet)
		{
			TRealtimeMeshBuilderLocal<uint16, FPackedNormal, FVector2DHalf, 1> Builder(StreamSet);

			for (int32 VertexIdx = 0; VertexIdx < Vertices.Num(); VertexIdx++)
			{				
				Builder.EditVertex(VertexIdx).SetPosition(FVector3f(NewVertices[VertexIdx].X, NewVertices[VertexIdx].Y, NewVertices[VertexIdx].Z));
			}

			TSet<FRealtimeMeshStreamKey> Res;
			Res.Add(FRealtimeMeshStreams::Position);
			
			return Res;
		}
	);
}

void UDeformableMeshComponent::SpawnCollisionNode(FVector Location) {
	UCollisionNodeComponent* CreatedCollisionNode = NewObject<UCollisionNodeComponent>(this);
	if (!CreatedCollisionNode) {
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("New object wasn't created!")));
	}

	CreatedCollisionNode->NodeId = CollisionNodesCount;
	CreatedCollisionNode->DeformableMesh = this;
	bool bResult = CreatedCollisionNode->AttachToComponent(this, FAttachmentTransformRules::KeepWorldTransform);
	CreatedCollisionNode->RegisterComponent();
	CreatedCollisionNode->SetSphereRadius(CageNodeRadius);
	CreatedCollisionNode->SetRelativeLocation(Location);
	CreatedCollisionNode->Location = Location;
	CreatedCollisionNode->SetHiddenInGame(!bIsDebug);

	CollisionNodesCount++;
	CollisionNodes.Push(CreatedCollisionNode);
}

void UDeformableMeshComponent::SpawnCollisionNodeWithPhysicsConstraint(FVector Location) {
	UCollisionNodeComponent* CreatedCollisionNode = NewObject<UCollisionNodeComponent>(this);
	if (!CreatedCollisionNode) {
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("New object wasn't created!")));
	}

	CreatedCollisionNode->NodeId = CollisionNodesCount;
	CreatedCollisionNode->DeformableMesh = this;
	CreatedCollisionNode->SetMassOverrideInKg(NAME_None, 25.0f);
	CreatedCollisionNode->AttachToComponent(this, FAttachmentTransformRules::KeepWorldTransform);
	CreatedCollisionNode->SetSphereRadius(CageNodeRadius);
	CreatedCollisionNode->SetHiddenInGame(!bIsDebug);
	CreatedCollisionNode->RegisterComponent();
	CreatedCollisionNode->SetRelativeLocation(Location);
	CreatedCollisionNode->Location = Location;

	CreatedCollisionNode->SetSimulatePhysics(true);

	UPhysicsConstraintComponent* ConstraintComp = NewObject<UPhysicsConstraintComponent>(this);
	if (!CreatedCollisionNode) {
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Constraint wasn't created!")));
	}

	ConstraintComp->AttachToComponent(this, FAttachmentTransformRules::KeepWorldTransform);
	ConstraintComp->SetDisableCollision(true);
	ConstraintComp->SetProjectionEnabled(false);

	// Constraint Limits
	ConstraintComp->SetLinearXLimit(ELinearConstraintMotion::LCM_Free, 0);
	ConstraintComp->SetLinearYLimit(ELinearConstraintMotion::LCM_Free, 0);
	ConstraintComp->SetLinearZLimit(ELinearConstraintMotion::LCM_Free, 0);
	ConstraintComp->SetAngularSwing1Limit(EAngularConstraintMotion::ACM_Locked, 0);
	ConstraintComp->SetAngularSwing2Limit(EAngularConstraintMotion::ACM_Locked, 0);
	ConstraintComp->SetAngularTwistLimit(EAngularConstraintMotion::ACM_Locked, 0);

	// Constraint Linear Position Force
	ConstraintComp->SetLinearPositionTarget(FVector(0, 0, 0));
	ConstraintComp->SetLinearPositionDrive(true, true, true);

	// Constraint Linear Velocity Force
	ConstraintComp->SetLinearVelocityTarget(FVector(0, 0, 0));
	ConstraintComp->SetLinearVelocityDrive(true, true, true);
	ConstraintComp->SetLinearDriveParams(25000, 2500, 0);

	ConstraintComp->RegisterComponent();
	ConstraintComp->SetWorldLocation(this->GetComponentLocation());
	ConstraintComp->SetConstrainedComponents(this, NAME_None, CreatedCollisionNode, NAME_None);

	CollisionNodesCount++;
	CollisionNodes.Push(CreatedCollisionNode);
	NodeConstraints.Push(ConstraintComp);
}

void UDeformableMeshComponent::GenerateCollisionModel() {
	TArray<FVector> CageMeshVertices;
	TArray<int32> CageMeshTriangles;
	TArray<FVector> CageMeshNormals;
	TArray<FVector2D> CageMeshUVs;
	TArray<FProcMeshTangent> CageMeshTangents;
	UKismetProceduralMeshLibrary::GetSectionFromStaticMesh(CageMesh, 0, 0, CageMeshVertices, CageMeshTriangles, CageMeshNormals, CageMeshUVs, CageMeshTangents);

	for (FVector VertexLocation : CageMeshVertices) {
		CageVertices.AddUnique(VertexLocation);
	}

	if (bUsePhysicsConstraint) {
		for (FVector Location : CageVertices) {
			SpawnCollisionNodeWithPhysicsConstraint(Location);
		}
	}
	else {
		for (FVector Location : CageVertices) {
			SpawnCollisionNode(Location);
		}
	}
	

	for (int32 Index : CageMeshTriangles) {
		CageTriangles.Add(CageVertices.Find(CageMeshVertices[Index]));
	}
}

void UDeformableMeshComponent::MoveNodes(int NodeId, FVector NormalImpulse, const FHitResult& Hit) {
	float SharedMass = 0.0f;
	auto EngineWorld = GetWorld();
	
	AActor* SelfActor = this->GetOwner()->GetRootComponent()->GetOwner();

	if (bUsePhysicsConstraint) {
		FVector NewTargetPosition = NodeConstraints[NodeId]->ConstraintInstance.GetLinearPositionTarget() + NormalImpulse / (-2500);
		//NodeConstraints[NodeId]->ConstraintInstance.SetLinearPositionTarget(NewTargetPosition);
	}
	else {
		for (UCollisionNodeComponent* CollisionNode : CollisionNodes) {
			FHitResult TraceHit = CollisionNode->LineTrace(EngineWorld, SelfActor, CollisionNode->GetComponentLocation() + Hit.ImpactNormal * -10, CollisionNode->GetComponentLocation() + Hit.ImpactNormal * -20, bIsDebug);
			if (TraceHit.bBlockingHit && Hit.GetComponent() == TraceHit.GetComponent()) {
				FVector Impulse = NormalImpulse / 100 * 0.01 * ((10 - Hit.Distance) / 10);
				if (Impulse.Length() < 1) continue;
				CollisionNode->AddWorldOffset(Impulse);
				CollisionNode->Location += Impulse;
			}
		}
	}

	UpdateMesh();
}

TArray<float> UDeformableMeshComponent::ComputeWeightsForVertex(FVector VertexLocation) {
	TArray<float> Weights = TArray<float>();
	Weights.SetNum(CageVertices.Num());

	const int32 NumVertices = CageVertices.Num();
	const int32 NumTriangles = CageTriangles.Num();

	TArray<float> Distances;
	TArray<FVector> Directions;
	Distances.SetNumZeroed(NumVertices);
	Directions.SetNum(NumVertices);

	// Initialize weights to zero
	for (int32 v = 0; v < Weights.Num(); ++v)
	{
		Weights[v] = 0.0f;
	}

	for (int32 v = 0; v < NumVertices; ++v)
	{
		FVector Delta = CageVertices[v] - VertexLocation;
		Distances[v] = Delta.Size();

		if (Distances[v] < Epsilon)
		{
			Weights[v] = 1.0f;
			return Weights;
		}

		Directions[v] = (Delta / Distances[v]);
	}

	for (int32 t = 0; t < NumTriangles; t += 3)
	{
		TArray<int32> FaceNodes;
		FaceNodes.Add(CageTriangles[t]);
		FaceNodes.Add(CageTriangles[t + 1]);
		FaceNodes.Add(CageTriangles[t + 2]);

		const FVector& FaceVertex0 = CageVertices[FaceNodes[0]];
		const FVector& FaceVertex1 = CageVertices[FaceNodes[1]];
		const FVector& FaceVertex2 = CageVertices[FaceNodes[2]];

		const float Lenght0 = FVector::Dist(Directions[FaceNodes[1]], Directions[FaceNodes[2]]);
		const float Lenght1 = FVector::Dist(Directions[FaceNodes[0]], Directions[FaceNodes[2]]);
		const float Lenght2 = FVector::Dist(Directions[FaceNodes[0]], Directions[FaceNodes[1]]);

		const float Theta0 = 2 * FMath::Asin(Lenght0 / 2);
		const float Theta1 = 2 * FMath::Asin(Lenght1 / 2);
		const float Theta2 = 2 * FMath::Asin(Lenght2 / 2);

		const float Half = (Theta0 + Theta1 + Theta2) / 2;

		if (FMath::Abs(PI - Half) < Epsilon)
		{
			Weights.Init(0, CageVertices.Num());
			Weights[FaceNodes[0]] = FMath::Sin(Theta0) * Distances[FaceNodes[1]] * Distances[FaceNodes[2]];
			Weights[FaceNodes[1]] = FMath::Sin(Theta1) * Distances[FaceNodes[2]] * Distances[FaceNodes[0]];
			Weights[FaceNodes[2]] = FMath::Sin(Theta2) * Distances[FaceNodes[0]] * Distances[FaceNodes[1]];
			break;
		}

		FVector N = FVector::CrossProduct(FaceVertex1 - FaceVertex0, FaceVertex2 - FaceVertex0).GetSafeNormal();
		float Determinant = FVector::DotProduct(FaceVertex0 - VertexLocation, N);

		float c0 = (2 * FMath::Sin(Half) * FMath::Sin(Half - Theta0)) / (FMath::Sin(Theta1) * FMath::Sin(Theta2)) - 1;
		float c1 = (2 * FMath::Sin(Half) * FMath::Sin(Half - Theta1)) / (FMath::Sin(Theta0) * FMath::Sin(Theta2)) - 1;
		float c2 = (2 * FMath::Sin(Half) * FMath::Sin(Half - Theta2)) / (FMath::Sin(Theta0) * FMath::Sin(Theta1)) - 1;

		float s0 = FMath::Sign(Determinant) * FMath::Sqrt(1 - c0 * c0);
		float s1 = FMath::Sign(Determinant) * FMath::Sqrt(1 - c1 * c1);
		float s2 = FMath::Sign(Determinant) * FMath::Sqrt(1 - c2 * c2);

		if (FMath::Abs(s0) < Epsilon || FMath::Abs(s1) < Epsilon || FMath::Abs(s2) < Epsilon)
			continue;

		Weights[FaceNodes[0]] += (Theta0 - c1 * Theta2 - c2 * Theta1) / (Distances[FaceNodes[0]] * FMath::Sin(Theta1) * s2);
		Weights[FaceNodes[1]] += (Theta1 - c2 * Theta0 - c0 * Theta2) / (Distances[FaceNodes[1]] * FMath::Sin(Theta2) * s0);
		Weights[FaceNodes[2]] += (Theta2 - c0 * Theta1 - c1 * Theta0) / (Distances[FaceNodes[2]] * FMath::Sin(Theta0) * s1);
	}

	float SumWeights = 0.0f;
	for (float Weight : Weights)
	{
		SumWeights += Weight;
	}

	for (float& Weight : Weights)
	{
		Weight /= SumWeights;
	}

	return Weights;
}

void UDeformableMeshComponent::ComputeWeights() {
	int32 VerticesCount = Vertices.Num();

	for (int32 i = 0; i < VerticesCount; ++i) {
		DeformationTransferWeights.Add(ComputeWeightsForVertex(Vertices[i]));
	}
}