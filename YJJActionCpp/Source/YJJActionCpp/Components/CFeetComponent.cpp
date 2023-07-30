#include "Components/CFeetComponent.h"
#include "Global.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"

#define LOG_UCFeetComponent

UCFeetComponent::UCFeetComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	Owner = Cast<ACCommonCharacter>(GetOwner());
}

void UCFeetComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UCFeetComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	float leftDistance, rightDistance;
	FRotator leftRotation, rightRotation;

	Trace(LeftSocket, leftDistance, leftRotation);
	Trace(RightSocket, rightDistance, rightRotation);

	const float offset = FMath::Min(leftDistance, rightDistance);

	Data.PelvisDistance.Z = 
		UKismetMathLibrary::FInterpTo(Data.PelvisDistance.Z, offset, DeltaTime, InterpSpeed);

	Data.LeftDistance.X = 
		UKismetMathLibrary::FInterpTo(Data.LeftDistance.X, (leftDistance - offset), DeltaTime, InterpSpeed);
	Data.RightDistance.X = 
		UKismetMathLibrary::FInterpTo(Data.RightDistance.X, -(rightDistance - offset), DeltaTime, InterpSpeed);

	Data.LeftRotation = 
		UKismetMathLibrary::RInterpTo(Data.LeftRotation, leftRotation, DeltaTime, InterpSpeed);
	Data.RightRotation = 
		UKismetMathLibrary::RInterpTo(Data.RightRotation, rightRotation, DeltaTime, InterpSpeed);

#ifdef LOG_UCFeetComponent
	CLog::Print(Data.LeftDistance, 11);
	CLog::Print(Data.RightDistance, 12);
	CLog::Print(Data.PelvisDistance, 13);
	CLog::Print(Data.LeftRotation, 14);
	CLog::Print(Data.RightRotation, 15);
#endif
}

void UCFeetComponent::Trace(const FName InName, float& OutDistance, FRotator& OutRotation) const
{
	const FVector socket = Owner->GetMesh()->GetSocketLocation(InName);

	float z = Owner->GetActorLocation().Z;

	const FVector start = FVector(socket.X, socket.Y, z);

	z = start.Z - Owner->GetCapsuleComponent()->GetScaledCapsuleHalfHeight() - TraceDistance;

	const FVector end = FVector(socket.X, socket.Y, z);

	TArray<AActor*> ignores;
	ignores.Add(Owner.Get());

	FHitResult hitResult;
	UKismetSystemLibrary::LineTraceSingle(
		GetWorld(), 
		start, end, 
		ETraceTypeQuery::TraceTypeQuery1, 
		true, 
		ignores, 
		DrawDebug, 
		hitResult, 
		true, 
		FLinearColor::Green, FLinearColor::Red);

	OutDistance = 0;
	OutRotation = FRotator::ZeroRotator;

	CheckFalse(hitResult.bBlockingHit);


	const float length = (hitResult.ImpactPoint - hitResult.TraceEnd).Size();
	OutDistance = length + OffsetDistance - TraceDistance;

	const float roll = UKismetMathLibrary::DegAtan2(hitResult.Normal.Y, hitResult.Normal.Z);
	const float pitch = -UKismetMathLibrary::DegAtan2(hitResult.Normal.X, hitResult.Normal.Z);

	OutRotation = FRotator(pitch, 0, roll);
}
