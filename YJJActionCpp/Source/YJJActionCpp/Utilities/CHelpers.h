#pragma once
#include "CoreMinimal.h"
#include "Particles/ParticleSystem.h"
#include "NiagaraSystem.h"
#include "NiagaraFunctionLibrary.h"
#include "Characters/CCommonCharacter.h"
#include "GameFramework/Controller.h"

#define CheckTrue(x) { if (x == true) return; }
#define CheckTrueResult(x, y) { if (x == true) return y; }

#define CheckFalse(x) { if(x == false) return;}
#define CheckFalseResult(x, y) { if(x == false) return y;}

#define CheckNull(x) { if(x == nullptr) return;}
#define CheckNullResult(x, y) { if(x == nullptr) return y;}

#define CreateTextRender()\
{\
	CHelpers::CreateComponent<UTextRenderComponent>(this, &Text, "Text", Root);\
	Text->SetRelativeLocation(FVector(0, 0, 100));\
	Text->SetRelativeRotation(FRotator(0, 100, 0));\
	Text->SetRelativeScale3D(FVector(2));\
	Text->TextRenderColor = FColor::Red;\
	Text->HorizontalAlignment = EHorizTextAligment::EHTA_Center;\
	Text->Text = FText::FromString(GetName().Replace(TEXT("Default__"), TEXT("")));\
}

class YJJACTIONCPP_API CHelpers
{
public:
	template<typename T>
	static void CreateComponent(AActor* InActor, T** OutComponent, const FName InName, 
		USceneComponent* InParent = nullptr, const FName InSocketName = NAME_None)
	{
		*OutComponent = InActor->CreateDefaultSubobject<T>(InName);

		if (!!InParent)
		{
			(*OutComponent)->SetupAttachment(InParent, InSocketName);	// 소켓명 언더바 대신 띄어쓰기 써야 함

			return;
		}

		InActor->SetRootComponent(*OutComponent);
	}

	template<typename T>
	static void CreateActorComponent(AActor* InActor, T** OutComponent, const FName InName)
	{
		*OutComponent = InActor->CreateDefaultSubobject<T>(InName);
	}

	template<typename T>
	static void GetAsset(T** OutObject, const FString InPath)
	{
		ConstructorHelpers::FObjectFinder<T> asset(*InPath);
		*OutObject = asset.Object;
	}

	template<typename T>
	static void GetAssetDynamic(T** OutObject, const FString InPath)
	{
		*OutObject = Cast<T>(StaticLoadObject(T::StaticClass(), nullptr, *InPath));
	}

	template<typename T>
	static void GetClass(TSubclassOf<T>* OutClass, const FString InPath)
	{
		ConstructorHelpers::FClassFinder<T> asset(*InPath);
		*OutClass = asset.Class;
	}

	template<typename T>
	static T* FindActor(const TWeakObjectPtr<UWorld> InWorld)
	{
		for (AActor* actor : InWorld->GetCurrentLevel()->Actors)
		{
			if (!!actor && actor->IsA<T>())
				return Cast<T>(actor);
		}

		return nullptr;
	}

	template<typename T>
	static void FindActors(const TWeakObjectPtr<UWorld> InWorld, TArray<T*>& OutActors)
	{
		for (AActor* actor : InWorld->GetCurrentLevel()->Actors)
		{
			if (!!actor && actor->IsA<T>())
				OutActors.Add(Cast<T>(actor));
		}
	}

	template<typename T>
	static T* GetComponent(AActor* InActor)
	{
		return Cast<T>(InActor->GetComponentByClass(T::StaticClass()));
	}

	template<typename T>
	static T* GetComponent(AActor* InActor, const FString& InName)
	{
		TArray<T*> components;
		InActor->GetComponents<T>(components);

		for (T* component : components)
		{
			if (component->GetName() == InName)
				return component;
		}

		return nullptr;
	}

	static void AttachTo(AActor* InActor, USceneComponent* InParent, const FName InSocketName)
	{
		InActor->AttachToComponent(InParent, 
			FAttachmentTransformRules(EAttachmentRule::KeepRelative, true), InSocketName);
	}

	template<typename EnumType>
	static FString ConvertEnumToString(const EnumType InValue)
	{
		return StaticEnum<EnumType>()->GetNameStringByValue(static_cast<int64>(InValue));
	}

	template<typename EnumType>
	static EnumType ConvertTCHARToEnum(const TCHAR& InValue)
	{
		int32 typeIndex = InValue - '0';
		EnumType weaponType = static_cast<EnumType>(typeIndex);
		return weaponType;
	}

	static void PlayEffect(const TWeakObjectPtr<UWorld> InWorld, const TWeakObjectPtr<UFXSystemAsset> InAsset,
		const FTransform& InTransform, const TWeakObjectPtr<USkeletalMeshComponent> InMesh = nullptr,
		FName InSocketName = NAME_None)
	{
		const TWeakObjectPtr<UParticleSystem> particle = Cast<UParticleSystem>(InAsset);
		const TWeakObjectPtr<UNiagaraSystem> niagara = Cast<UNiagaraSystem>(InAsset);

		FVector location = InTransform.GetLocation();
		FRotator rotation = FRotator(InTransform.GetRotation());
		FVector scale = InTransform.GetScale3D();

		if (!!InMesh.Get())
		{
			if (!!particle.Get())
			{
				UGameplayStatics::SpawnEmitterAttached(particle.Get(), InMesh.Get(), InSocketName, location, rotation, scale);
				return;
			}

			if (!!niagara.Get())
			{
				UNiagaraFunctionLibrary::SpawnSystemAttached(niagara.Get(), InMesh.Get(), InSocketName, location, rotation, scale,
					EAttachLocation::KeepRelativeOffset,true, ENCPoolMethod::None);
				return;
			}
		}

		if (!!particle.Get())
		{
			UGameplayStatics::SpawnEmitterAtLocation(InWorld.Get(), particle.Get(), InTransform);
			return;
		}

		if (!!niagara.Get())
		{
			UNiagaraFunctionLibrary::SpawnSystemAtLocation(InWorld.Get(), niagara.Get(), location, rotation, scale);
			return;
		}
	}

	template<typename T>
	static void LoadAsset(T** OutObject, const FString& Path)
	{
		*OutObject = Cast<T>(StaticLoadObject(T::StaticClass(), nullptr, *Path));
	}

	static TWeakObjectPtr<ACCommonCharacter> GetNearlyFromAngle(
		TWeakObjectPtr<ACCommonCharacter> InCenter,
		TArray<TWeakObjectPtr<ACCommonCharacter>> InArray,
		TWeakObjectPtr<AController> InController)
	{
		CheckTrueResult(InArray.Num() <= 0, nullptr);

		const float limitLeastAngle = 0.7f;
		float maxAngle = 0.0f;
		TWeakObjectPtr<ACCommonCharacter> outTarget;

		for (const auto& elem : InArray)
		{
			if (!!elem.IsValid())
			{
				FVector diff = (elem->GetActorLocation() - InCenter->GetActorLocation());
				diff.Normalize();

				FVector forward = UKismetMathLibrary::GetForwardVector(InController->GetControlRotation());

				float curAngle = diff.DotProduct(diff, forward);

				if (curAngle > limitLeastAngle &&
					curAngle > maxAngle)
				{
					curAngle = maxAngle;
					outTarget = elem;
				}
			}
		}

		return outTarget;
	}
};