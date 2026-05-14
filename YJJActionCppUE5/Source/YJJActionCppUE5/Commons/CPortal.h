#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CPortal.generated.h"

class USphereComponent;
class USoundBase;
class UUserWidget;
class UStaticMesh;
class UStaticMeshComponent;
class UNiagaraSystem;
class UNiagaraComponent;
class UMaterialInterface;

// BP_Portal 과 동일한 컴포넌트 오브젝트 이름(SphereComponent_2, NiagaraComponent_0, StaticMeshComponent_12)을 쓴다.
UCLASS(Blueprintable)
class YJJACTIONCPPUE5_API ACPortal : public AActor
{
	GENERATED_BODY()

public:
	ACPortal();

protected:
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	// BP_Portal 등에 남은 Sphere / Niagara / StaticMesh 변수명과 UPROPERTY 이름이 겹치면 SKEL 컴파일이 실패하므로
	// 네이티브 멤버 이름은 구체적으로 둔다. 블루프린트 API는 GetSphere() 등으로 동일 유지.
	UFUNCTION(BlueprintPure, Category = "Portal")
	USphereComponent* GetSphere() const { return OverlapSphere.Get(); }

	UFUNCTION(BlueprintPure, Category = "Portal")
	UNiagaraComponent* GetNiagara() const { return PortalNiagara.Get(); }

	UFUNCTION(BlueprintPure, Category = "Portal")
	UStaticMeshComponent* GetStaticMesh() const { return PortalMesh.Get(); }

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USphereComponent> OverlapSphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UNiagaraComponent> PortalNiagara;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> PortalMesh;

private:
	UFUNCTION()
	void OnSphereOverlapBegin(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

	void OnLoadingDelayElapsed();

	void ApplyPortalVisualDefaults();

	UPROPERTY(EditAnywhere, Category = "Portal|Collision", meta = (ClampMin = "1.0"))
	float SphereRadius = 100.0f;

	UPROPERTY(EditAnywhere, Category = "Portal|Visual")
	TObjectPtr<UStaticMesh> Mesh;

	UPROPERTY(EditAnywhere, Category = "Portal|Visual")
	TObjectPtr<UMaterialInterface> MeshMaterial;

	UPROPERTY(EditAnywhere, Category = "Portal|Visual")
	TObjectPtr<UNiagaraSystem> NiagaraSystem;

	UPROPERTY(EditAnywhere, Category = "Portal|Visual")
	bool bAutoActivateNiagara = true;

	UPROPERTY(EditAnywhere, Category = "Portal|Audio")
	TObjectPtr<USoundBase> LoopSound;

	UPROPERTY(EditAnywhere, Category = "Portal|Travel")
	FName NextLevelName;

	UPROPERTY(EditAnywhere, Category = "Portal|Travel")
	TSubclassOf<UUserWidget> LoadingWidgetClass;

	UPROPERTY(EditAnywhere, Category = "Portal|Travel", meta = (ClampMin = "0.0"))
	float TravelDelaySeconds = 2.0f;

	FTimerHandle TravelDelayTimer;

	bool bTravelPending = false;
};
