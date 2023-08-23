#include "Objects/CProceduralObject.h"
#include "Global.h"
#include "ProceduralMeshComponent.h"

ACProceduralObject::ACProceduralObject()
{
	YJJHelpers::CreateComponent<UProceduralMeshComponent>(this, &MeshComp, "MeshComp");

	const FVector p = FVector(0.5f, 0.5f, 0.5f);


	//Front
	Positions.Add(FVector(-p.X, -p.Y, -p.Z));
	Positions.Add(FVector(-p.X, -p.Y, +p.Z));
	Positions.Add(FVector(-p.X, +p.Y, -p.Z));
	Positions.Add(FVector(-p.X, +p.Y, +p.Z));

	for (int32 i = 0; i < 4; i++)
	{
		Colors.Add(FColor(128, 0, 0, 255));
		Normals.Add(FVector(-1, 0, 0));
	}
	AddTriangles(0);


	//Back
	Positions.Add(FVector(+p.X, +p.Y, -p.Z));
	Positions.Add(FVector(+p.X, +p.Y, +p.Z));
	Positions.Add(FVector(+p.X, -p.Y, -p.Z));
	Positions.Add(FVector(+p.X, -p.Y, +p.Z));

	for (int32 i = 0; i < 4; i++)
	{
		Colors.Add(FColor(0, 128, 0, 255));
		Normals.Add(FVector(+1, 0, 0));
	}
	AddTriangles(4);


	//Top
	Positions.Add(FVector(-p.X, -p.Y, +p.Z));
	Positions.Add(FVector(+p.X, -p.Y, +p.Z));
	Positions.Add(FVector(-p.X, +p.Y, +p.Z));
	Positions.Add(FVector(+p.X, +p.Y, +p.Z));

	for (int32 i = 0; i < 4; i++)
	{
		Colors.Add(FColor(0, 0, 128, 255));
		Normals.Add(FVector(0, 0, +1));
	}
	AddTriangles(8);


	//Bottom
	Positions.Add(FVector(-p.X, -p.Y, -p.Z));
	Positions.Add(FVector(-p.X, +p.Y, -p.Z));
	Positions.Add(FVector(+p.X, -p.Y, -p.Z));
	Positions.Add(FVector(+p.X, +p.Y, -p.Z));

	for (int32 i = 0; i < 4; i++)
	{
		Colors.Add(FColor(0, 128, 128, 255));
		Normals.Add(FVector(0, 0, -1));
	}
	AddTriangles(12);


	//Left
	Positions.Add(FVector(+p.X, -p.Y, -p.Z));
	Positions.Add(FVector(+p.X, -p.Y, +p.Z));
	Positions.Add(FVector(-p.X, -p.Y, -p.Z));
	Positions.Add(FVector(-p.X, -p.Y, +p.Z));

	for (int32 i = 0; i < 4; i++)
	{
		Colors.Add(FColor(128, 0, 128, 255));
		Normals.Add(FVector(0, -1, 0));
	}
	AddTriangles(16);


	//Right
	Positions.Add(FVector(-p.X, +p.Y, -p.Z));
	Positions.Add(FVector(-p.X, +p.Y, +p.Z));
	Positions.Add(FVector(+p.X, +p.Y, -p.Z));
	Positions.Add(FVector(+p.X, +p.Y, +p.Z));

	for (int32 i = 0; i < 4; i++)
	{
		Colors.Add(FColor(128, 128, 128, 255));
		Normals.Add(FVector(0, +1, 0));
	}
	AddTriangles(20);



	for (int32 i = 0; i < Positions.Num() / 4; i++)
	{
		Uvs.Add(FVector2D(0, 1));
		Uvs.Add(FVector2D(0, 0));
		Uvs.Add(FVector2D(1, 1));
		Uvs.Add(FVector2D(1, 0));
	}


	MeshComp->CreateMeshSection(0, Positions, Indices, Normals, Uvs, Colors, TArray<FProcMeshTangent>(), true);
	MeshComp->SetRelativeScale3D(FVector(100));
}

void ACProceduralObject::AddTriangles(const int32 InStart)
{
	Indices.Add(InStart + 2);
	Indices.Add(InStart + 1);
	Indices.Add(InStart + 0);

	Indices.Add(InStart + 3);
	Indices.Add(InStart + 1);
	Indices.Add(InStart + 2);
}