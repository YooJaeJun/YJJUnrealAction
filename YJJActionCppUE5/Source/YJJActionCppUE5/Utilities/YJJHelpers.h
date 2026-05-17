#pragma once
#include "CoreMinimal.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Utilities/CLog.h"
#include "Particles/ParticleSystem.h"
#include "NiagaraSystem.h"
#include "NiagaraFunctionLibrary.h"
#include "Characters/CCommonCharacter.h"
#include "Commons/CGameMode.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/Controller.h"
#include "Engine/DataTable.h"
#include "Weapons/AddOns/CMotionTrail.h"

//#define CheckTrue(x) { if (true == (x)) { CLog::Log("CheckTrue"); return; }}
//#define CheckTrueResult(x, y) { if (true == (x)) { CLog::Log("CheckTrueResult"); return (y); }}
//
//#define CheckFalse(x) { if(false == (x)) { CLog::Log("CheckFalse"); return; }}
//#define CheckFalseResult(x, y) { if(false == (x)) { CLog::Log("CheckFalseResult"); return (y); }}
//
//#define CheckNull(x) { if(nullptr == (x)) { CLog::Log("CheckNull"); return; }}
//#define CheckNullResult(x, y) { if(nullptr == (x)) { CLog::Log("CheckNullResult"); return (y); }}

#define CheckTrue(x) { if (true == (x)) return; }
#define CheckTrueResult(x, y) { if (true == (x)) return (y); }

#define CheckFalse(x) { if(false == (x)) return; }
#define CheckFalseResult(x, y) { if(false == (x)) return (y); }

#define CheckNull(x) { if(nullptr == (x)) return; }
#define CheckNullResult(x, y) { if(nullptr == (x)) return (y); }

#define CheckRefNull(x, y) { if (nullptr == &(x)) { CLog::Log((y)); return; }}
#define CheckRefNullResult(x, y, z) { if (nullptr == &(x)) { CLog::Log((y)); return (z); }}

#define CreateTextRender()\
{\
	YJJHelpers::CreateComponent<UTextRenderComponent>(this, &Text, "Text", Root);\
	Text->SetRelativeLocation(FVector(0, 0, 100));\
	Text->SetRelativeRotation(FRotator(0, 100, 0));\
	Text->SetRelativeScale3D(FVector(2));\
	Text->TextRenderColor = FColor::Red;\
	Text->HorizontalAlignment = EHorizTextAligment::EHTA_Center;\
	Text->Text = FText::FromString(GetName().Replace(TEXT("Default__"), TEXT("")));\
}

class YJJACTIONCPPUE5_API YJJHelpers
{
public:
	static int32 Npos() { return -1; }

	template<typename T>
	static void CreateComponent(
		TObjectPtr<AActor> InActor,
		TObjectPtr<T>* OutComponent,
		const FName InName, 
		TObjectPtr<USceneComponent> InParent = nullptr,
		const FName InSocketName = NAME_None)
	{
		*OutComponent = InActor->CreateDefaultSubobject<T>(InName);

		if (IsValid(InParent))
		{
			(*OutComponent)->SetupAttachment(InParent, InSocketName);	// 에디터 에셋 소켓명은 언더바가 아니라 공백을 쓰는 경우가 있다.

			return;
		}

		InActor->SetRootComponent(*OutComponent);
	}

	template<typename T>
	static void CreateActorComponent(TObjectPtr<AActor> InActor, TObjectPtr<T>* OutComponent, const FName InName)
	{
		*OutComponent = InActor->CreateDefaultSubobject<T>(InName);
	}

	template<typename T>
	static void GetAsset(TObjectPtr<T>* OutObject, const FString InPath)
	{
		const ConstructorHelpers::FObjectFinder<T> asset(*InPath);
		*OutObject = asset.Object;
	}

	// USTRUCT 등 TObjectPtr 대신 U* 멤버에 쓸 때(예: FHitData::Montage).
	template<typename T>
	static void GetAsset(T** OutObject, const FString InPath)
	{
		const ConstructorHelpers::FObjectFinder<T> asset(*InPath);
		*OutObject = asset.Object;
	}

	template<typename T>
	static void GetAssetDynamic(TObjectPtr<T>* OutObject, const FString InPath)
	{
		*OutObject = Cast<T>(StaticLoadObject(T::StaticClass(), nullptr, *InPath));
	}

	template<typename T>
	static void GetAssetDynamic(T** OutObject, const FString InPath)
	{
		*OutObject = Cast<T>(StaticLoadObject(T::StaticClass(), nullptr, *InPath));
	}

	template<typename T>
	static void GetClass(TSubclassOf<T>* OutClass, const FString InPath)
	{
		const ConstructorHelpers::FClassFinder<T> asset(*InPath);
		*OutClass = asset.Class;
	}

	/** CDO/ctor 동기 블프 로드는 순환 패키지·AsyncLoading2 Phase 와 충돌할 수 있다 — 게임 시작 후 보충 용도. */
	template<typename T>
	static void GetClassDynamic(TSubclassOf<T>* OutClass, const FString& InPath)
	{
		UClass* const loaded = StaticLoadClass(T::StaticClass(), nullptr, *InPath, nullptr, LOAD_None, nullptr);
		*OutClass = loaded;
	}

	template<typename T>
	static T* FindActor(const TWeakObjectPtr<UWorld> InWorld)
	{
		for (TObjectPtr<AActor> actor : InWorld->GetCurrentLevel()->Actors)
		{
			if (IsValid(actor) && actor->IsA<T>())
				return Cast<T>(actor);
		}

		return nullptr;
	}

	template<typename T>
	static void FindActors(const TWeakObjectPtr<UWorld> InWorld, TArray<TObjectPtr<T>>& OutActors)
	{
		for (TObjectPtr<AActor> actor : InWorld->GetCurrentLevel()->Actors)
		{
			if (IsValid(actor) && actor->IsA<T>())
				OutActors.Add(Cast<T>(actor));
		}
	}

	template<typename T>
	static TObjectPtr<T> GetComponent(TObjectPtr<AActor> InActor)
	{
		return Cast<T>(InActor->GetComponentByClass(T::StaticClass()));
	}

	template<typename T>
	static TObjectPtr<T> GetComponent(TObjectPtr<AActor> InActor, const FString& InName)
	{
		TArray<TObjectPtr<T>> components;
		InActor->GetComponents<T>(components);

		for (T* component : components)
		{
			if (component->GetName() == InName)
				return component;
		}

		return nullptr;
	}

	static void AttachTo(TObjectPtr<AActor> InActor, TObjectPtr<USceneComponent> InParent, const FName InSocketName)
	{
		InActor->AttachToComponent(
			InParent, 
			FAttachmentTransformRules(EAttachmentRule::KeepRelative, true), 
			InSocketName);
	}

	template<typename EnumType>
	static FString ConvertEnumToString(const EnumType InValue)
	{
		return StaticEnum<EnumType>()->GetNameStringByValue(static_cast<int64>(InValue));
	}

	template<typename EnumType>
	static EnumType ConvertTCHARToEnum(const TCHAR& InValue)
	{
		const int32 typeIndex = InValue - '0';
		EnumType weaponType = static_cast<EnumType>(typeIndex);
		return weaponType;
	}

	static void PlayEffect(const TWeakObjectPtr<UWorld> InWorld, const TWeakObjectPtr<UFXSystemAsset> InAsset,
		const FTransform& InTransform, const TWeakObjectPtr<USkeletalMeshComponent> InMesh = nullptr,
		FName InSocketName = NAME_None)
	{
		const TWeakObjectPtr<UParticleSystem> particle = Cast<UParticleSystem>(InAsset.Get());
		const TWeakObjectPtr<UNiagaraSystem> niagara = Cast<UNiagaraSystem>(InAsset.Get());

		const FVector location = InTransform.GetLocation();
		const FRotator rotation = FRotator(InTransform.GetRotation());
		const FVector scale = InTransform.GetScale3D();

		if (InMesh.IsValid())
		{
			if (particle.IsValid())
			{
				UGameplayStatics::SpawnEmitterAttached(particle.Get(), InMesh.Get(), InSocketName, location, rotation, scale);
				return;
			}

			if (niagara.IsValid())
			{
				UNiagaraFunctionLibrary::SpawnSystemAttached(niagara.Get(), InMesh.Get(), InSocketName, location, rotation, scale,
					EAttachLocation::KeepRelativeOffset,true, ENCPoolMethod::None);
				return;
			}
		}

		if (particle.IsValid())
		{
			UGameplayStatics::SpawnEmitterAtLocation(InWorld.Get(), particle.Get(), InTransform);
			return;
		}

		if (niagara.IsValid())
		{
			UNiagaraFunctionLibrary::SpawnSystemAtLocation(InWorld.Get(), niagara.Get(), location, rotation, scale);
			return;
		}
	}

	static UCUserWidget_HUD* GetHud(TWeakObjectPtr<ACCommonCharacter> InOwner)
	{
		if (false == InOwner.IsValid())
		{
			CLog::Log(TEXT("[UI] YJJHelpers::GetHud: InOwner 무효"));
			return nullptr;
		}

		const TWeakObjectPtr<ACGameMode> gameMode = Cast<ACGameMode>(UGameplayStatics::GetGameMode(InOwner->GetWorld()));
		if (false == gameMode.IsValid())
		{
			CLog::Log(FString::Printf(TEXT("[UI] YJJHelpers::GetHud: ACGameMode 없음 — Character=%s"), *InOwner->GetName()));
			return nullptr;
		}

		UCUserWidget_HUD* hud = gameMode->GetHUD();
		if (nullptr == hud)
		{
			CLog::Log(FString::Printf(TEXT("[UI] YJJHelpers::GetHud: GameMode->GetHUD() 무효 — Character=%s"), *InOwner->GetName()));
			return nullptr;
		}

		return hud;
	}

	template<typename T>
	static T* FindRow(TObjectPtr<UDataTable> table, int32 InValue, const TCHAR* contextString = TEXT(""))
	{
		return table->FindRow<T>(*FString::FromInt(InValue), contextString);
	}

	//////////////////
	// Sub Features //
	//////////////////

	static TWeakObjectPtr<ACCommonCharacter> GetNearForward(
		TWeakObjectPtr<ACCommonCharacter> InCenter,
		TArray<TWeakObjectPtr<ACCommonCharacter>> InArray,
		TWeakObjectPtr<AController> InController)
	{
		CheckTrueResult(InArray.Num() <= 0, nullptr);

		constexpr float limitLeastAngle = 0.7f;
		float bestDotSeen = limitLeastAngle;
		TWeakObjectPtr<ACCommonCharacter> outTarget;

		for (const TWeakObjectPtr<ACCommonCharacter>& otherCharacter : InArray)
		{
			if (false == otherCharacter.IsValid())
				continue;

			FVector diff = (otherCharacter->GetActorLocation() - InCenter->GetActorLocation());
			diff.Normalize();

			const FVector forward = UKismetMathLibrary::GetForwardVector(InController->GetControlRotation());

			const float dotToForward = FVector::DotProduct(diff, forward);

			// 같은 시야 콘이면 각도(dot) 더 큰 쪽을 남김 — 기존 maxAngle 이름이 실제 의미 없이 const 에 대입하던 버그 수정.
			if (dotToForward > limitLeastAngle && dotToForward > bestDotSeen)
			{
				bestDotSeen = dotToForward;
				outTarget = otherCharacter;
			}
		}

		return outTarget;
	}

	static float GetLook(FVector InForward, FVector InToTarget, FVector InUp)
	{
		return UKismetMathLibrary::Dot_VectorVector(
			InUp, 
			UKismetMathLibrary::Cross_VectorVector(InForward, InToTarget));
	}

	static void AddNearSideCharacters(
		TWeakObjectPtr<ACCommonCharacter> InCenter,
		TArray<TWeakObjectPtr<ACCommonCharacter>> InArray,
		TWeakObjectPtr<AController> InController,
		TMap<float, TWeakObjectPtr<ACCommonCharacter>>& OutNearCharacters)
	{
		CheckTrue(InArray.Num() <= 0);

		for (const TWeakObjectPtr<ACCommonCharacter>& otherCharacter : InArray)
		{
			if (otherCharacter.IsValid())
			{
				FVector toTarget = (otherCharacter->GetActorLocation() - InCenter->GetActorLocation());
				toTarget.Normalize();

				const FVector forward = UKismetMathLibrary::GetForwardVector(InController->GetControlRotation());

				const FVector up = InCenter->GetActorUpVector();

				const float direction = GetLook(forward, toTarget, up);

				OutNearCharacters.Add(direction, otherCharacter);
			}
		}
	}

	static ACMotionTrail* PlayMotionTrail(
		TSubclassOf<ACMotionTrail>& InClass,
		const TWeakObjectPtr<ACCommonCharacter> InOwner)
	{
		CheckNullResult(InClass, nullptr);
		CheckNullResult(InOwner, nullptr);

		FActorSpawnParameters params;
		params.Owner = InOwner.Get();
		params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		FVector location = InOwner->GetActorLocation();
		location.Z -= InOwner->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();

		FTransform transform;
		transform.SetTranslation(location);

		return InOwner->GetWorld()->SpawnActor<ACMotionTrail>(InClass, transform, params);
	}
};