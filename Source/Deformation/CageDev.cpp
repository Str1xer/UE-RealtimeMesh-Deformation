#include "CageDev.h"

TArray<float> UCageDev::ComputeWeights(
    FVector Point,
    TArray<FVector> CageVertices,
    TArray<int32> CageFaces
) {
    TArray<float> Weights = TArray<float>();
    Weights.SetNum(CageVertices.Num());

    const float Epsilon = 1e-2f;
    const int32 NumVertices = CageVertices.Num();
    const int32 NumTriangles = CageFaces.Num();

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
        FVector Delta = CageVertices[v] - Point;
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
        FaceNodes.Add(CageFaces[t]);
        FaceNodes.Add(CageFaces[t + 1]);
        FaceNodes.Add(CageFaces[t + 2]);

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
        float Determinant = FVector::DotProduct(FaceVertex0 - Point, N);

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
