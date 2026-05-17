#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Weapons/CWeaponStructures.h"
#include "CAnimalWeapon.generated.h"

class ACAnimal;
class ACCommonCharacter;
class UDataTable;
class USceneComponent;
class USphereComponent;
class UShapeComponent;
class USoundBase;

/**
 * 레거시 `/Game/Character/Animals/AnimalWeapon` — 야수 AI 무기 부착·도형 트레이스·히트표 적재까지 C++ 에서 처리한다.
 */
UCLASS(Blueprintable)
class YJJACTIONCPPUE5_API ACAnimalWeapon : public AActor
{
	GENERATED_BODY()

public:
	ACAnimalWeapon();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	/** 레거시 Owner 캐스트 대상(AI 소유 무기 동기화)·부착·동그룹 무시 검사 등에 사용. */
	UPROPERTY(
		EditAnywhere,
		BlueprintReadWrite,
		Category = "Animal|Weapon|Settings",
		meta = (MultiLine = "true", DisplayName = "Character"))
	TObjectPtr<ACAnimal> CharacterOwner;

	// 레거시 AnimalWeapon·BTTask_Animal_Action 등 — 블프 프로퍼티명 InAction 과 일치(DisplayName 과 동일).
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animal|Weapon|Actions", meta = (MultiLine = "true", DisplayName = "In Action"))
	bool InAction = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animal|Weapon|Hit", meta = (MultiLine = "true"))
	TArray<TObjectPtr<UShapeComponent>> Collisions;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animal|Weapon|Hit", meta = (MultiLine = "true"))
	TArray<TObjectPtr<ACCommonCharacter>> Hitted;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animal|Weapon|Sounds", meta = (MultiLine = "true", DisplayName = "Action Sound"))
	TObjectPtr<USoundBase> ActionSoundAsset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animal|Weapon|Sounds", meta = (MultiLine = "true", DisplayName = "Hit Sound"))
	TObjectPtr<USoundBase> HitSoundAsset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animal|Weapon|DataTables", meta = (MultiLine = "true", DisplayName = "Hit Datas"))
	TArray<FHitData> HitDatas;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animal|Weapon|DataTables", meta = (MultiLine = "true", DisplayName = "Hit Data Table"))
	TObjectPtr<UDataTable> HitDataTable;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animal|Weapon|DataTables", meta = (MultiLine = "true", DisplayName = "Do Action Datas"))
	TArray<FDoActionData> DoActionDatas;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animal|Weapon|DataTables", meta = (MultiLine = "true", DisplayName = "Do Action Data Table"))
	TObjectPtr<UDataTable> DoActionDataTable;

	/** `TryGetHitPointBetweenPrimitives` 구체 반경 — 레거시 BF_Helpers::GetHitPoint 와 무기 계열 디폴트에 맞춤. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animal|Weapon|Hit")
	float BetweenPrimitivesHitSphereRadius = 48.0f;

	/** 레거시 `OnBeginOverlap` 가 `HitDatas[0]` 에 고정되어 있던 것과 같게 디폴트 0이다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animal|Weapon|Hit")
	int32 HitApplyRowIndex = 0;

	/** ANS_Collision 등에서 `TryDispatchLegacyMainWeaponCollisionToggle` 이 ProcessEvent 하는 이름과 동형. */
	UFUNCTION(BlueprintCallable, Category = "Animal|Weapon|Collision")
	void OnCollisions();

	UFUNCTION(BlueprintCallable, Category = "Animal|Weapon|Collision")
	void OffCollisions();

	/** 레거시 `Begin_DoAction` — 블프는 노드 비어 있음(확장 훅). */
	UFUNCTION(BlueprintCallable, Category = "Animal|Weapon|Act")
	void Begin_DoAction();

	/** 레거시 `End_DoAction` — InAction 플래그 해제만 수행했다. */
	UFUNCTION(BlueprintCallable, Category = "Animal|Weapon|Act")
	void End_DoAction();

	/**
	 * 레거시 DoAction — InAction 참, 소유 동물 회전(InTarget 바라보기), 무작위 AttackAnim 재생,
	 * ActionSound 무기 위치 재생.(블프 후반의 RInterp 회전 브랜치는 exec 미연결·스킵)
	 */
	UFUNCTION(BlueprintCallable, Category = "Animal|Weapon|Act")
	void DoAction(FVector const& InTargetLocation);

	UFUNCTION(
		BlueprintCallable,
		BlueprintNativeEvent,
		Category = "Animal|Weapon|Hit",
		meta = (DisplayName = "On Begin Overlap"))
	void OnBeginOverlap(ACCommonCharacter* OtherCharacter, FVector const& HitPoint);
	virtual void OnBeginOverlap_Implementation(ACCommonCharacter* OtherCharacter, FVector const& HitPoint);

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Animal|Weapon|Components")
	TObjectPtr<USceneComponent> DefaultSceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Animal|Weapon|Components")
	TObjectPtr<USphereComponent> SphereOverlapForNavObstacle;

private:
	static FName AnimalWeaponAttachSocket();

	void Animal_RuntimeRefreshOwnerCharacterFromOverlapContext();
	bool Animal_HasAuthorityViaOwner() const;

	void Animal_RuntimeAttachSelfToAnimalMeshWeaponSocket();
	void Animal_RuntimeCollectCollisionShapesMatchingBp();
	void Animal_RuntimeBindOverlapDelegates();
	void Animal_RuntimeUnbindOverlapDelegates();

	void Animal_LoadTablesAllRows_LoadHitDatas();
	void Animal_LoadTablesAllRows_LoadDoActionDatas();

	UFUNCTION()
	void Animal_OnShapeBeginOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

	bool Animal_TraceHitPointBetween(
		UPrimitiveComponent* OverlappedPrim,
		UPrimitiveComponent* OtherPrim,
		FVector& OutHitPointWorld) const;
};
