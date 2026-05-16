#include "Characters/Animals/CAnimInstance_Animal.h"
#include "Characters/Animals/CAnimalAnimDynamicInfo.h"
#include "Characters/CCommonCharacter.h"
#include "Global.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"

void UCAnimInstance_Animal::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
	EnsureInfoActor();
}

void UCAnimInstance_Animal::NativeBeginPlay()
{
	Super::NativeBeginPlay();
	// NativeInitializeAnimation 시점에는 Pawn 이 아직 없을 수 있음.
	EnsureInfoActor();
}

void UCAnimInstance_Animal::NativeUninitializeAnimation()
{
	if (bSpawnedInfo && IsValid(Info))
	{
		Info->Destroy();
		Info = nullptr;
		bSpawnedInfo = false;
	}
	Super::NativeUninitializeAnimation();
}

void UCAnimInstance_Animal::NativeUpdateAnimation(const float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	APawn* pawn = TryGetPawnOwner();
	if (IsValid(Info) && IsValid(pawn))
		Info->Tick_Info(pawn);

	if (Owner.IsValid())
		bRotating = Owner->bIsRotating;

	CheckNull(Owner.Get());
}

void UCAnimInstance_Animal::EnsureInfoActor()
{
	if (IsValid(Info))
		return;

	APawn* pawn = TryGetPawnOwner();
	UWorld* world = GetWorld();
	if (false == IsValid(pawn) || false == IsValid(world))
		return;

	TSubclassOf<ACAnimalAnimDynamicInfo> spawnClass = InfoClass;
	// TSubclassOf 과 bool 과의 == 는 연산 후보 매칭이 실패함 (UE5 MSVC). 무효일 때 부모 디폴트로 대체한다.
	if (nullptr == spawnClass.Get())
		spawnClass = ACAnimalAnimDynamicInfo::StaticClass();

	FActorSpawnParameters params;
	params.Owner = pawn;
	params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::Undefined;

	ACAnimalAnimDynamicInfo* spawned = world->SpawnActor<ACAnimalAnimDynamicInfo>(
		spawnClass,
		pawn->GetActorTransform(),
		params);
	Info = spawned;
	if (IsValid(Info))
		bSpawnedInfo = true;
}
