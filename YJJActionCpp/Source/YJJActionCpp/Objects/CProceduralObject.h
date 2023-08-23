#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CProceduralObject.generated.h"

class UProceduralMeshComponent;

UCLASS()
class YJJACTIONCPP_API ACProceduralObject : public AActor
{
	GENERATED_BODY()

public:	
	ACProceduralObject();

private:
	void AddTriangles(const int32 InStart);

private:
	UPROPERTY(VisibleAnywhere)
		UProceduralMeshComponent* MeshComp;

private:
	TArray<FVector> Positions;
	TArray<int32> Indices;
	TArray<FVector> Normals;
	TArray<FColor> Colors;
	TArray<FVector2D> Uvs;
};
