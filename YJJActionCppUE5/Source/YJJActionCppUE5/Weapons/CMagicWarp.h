#pragma once

#include "CoreMinimal.h"
#include "Weapons/CSkillWeapon.h"
#include "Engine/HitResult.h"
#include "CMagicWarp.generated.h"

class UDecalComponent;
class UMaterialInterface;
class UParticleSystem;
class UParticleSystemComponent;
class USceneComponent;
class APlayerController;

/**
 * 레거시 `/Game/Magics/Warp/Magic_Warp` — ACMagic 계열.
 * 프리뷰용 Decal·Particle 은 기본 비가시·티크 꺼진 상태(블루프린트 디폴트와 동일 경향).
 */
UCLASS(Blueprintable)
class YJJACTIONCPPUE5_API ACMagicWarp : public ACMagic
{
	GENERATED_BODY()

public:
	ACMagicWarp();

protected:
	virtual void BeginPlay() override;

	virtual void Equip_Implementation() override;
	virtual void Unequip_Implementation() override;
	virtual void DoAction_Implementation(CEAttackType InAttackType, int32 InSkillIndex) override;
	virtual void Begin_DoAction_Implementation(CEAttackType InAttackType) override;

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Magic|Warp|Components")
	TObjectPtr<USceneComponent> WarpRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Magic|Warp|Components")
	TObjectPtr<UParticleSystemComponent> WarpParticle;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Magic|Warp|Components")
	TObjectPtr<UDecalComponent> WarpDecal;

	/** 에디터 기본값 `/Game/Assets/.../P_Smoke_Warp` 등 — 비어 있으면 컴포넌트 템플릿 유지 안 함. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Magic|Warp|Visual")
	TObjectPtr<UParticleSystem> WarpParticleTemplate;

	/** `/Game/Materials/Warp/M_Cursor_Inst` 류 — 비어 있으면 데칼 머티리얼 유지 안 함. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Magic|Warp|Visual")
	TObjectPtr<UMaterialInterface> WarpDecalMaterial;

	/**
	 * 레거시 `PlayerController` 변수.
	 * 비어 있으면 매직 컨텍스트 `Controller` 에서 APlayerController 로 캐스트해 사용.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Magic|Warp")
	TObjectPtr<APlayerController> WarpPlayerController;

	/** 레거시 `Location` — 워프 후보 좌표(SetCandidate 가 갱신). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Magic|Warp", meta = (DisplayName = "Location"))
	FVector WarpCandidateLocation = FVector::ZeroVector;

	/**
	 * 커서 라인트레이스(TraceTypeQuery1, Complex) → 차단 히트 시 위치·법선 기반 회전.
	 * PC 없음: OutHit=false 및 제로 반환(저빈도이므로 로그 1회 출력).
	 */
	UFUNCTION(
		BlueprintCallable,
		Category = "Magic|Warp|Utility",
		meta = (DisplayName = "Get Cursor Location And Rotation"))
	void GetCursorLocationAndRotation(bool& OutHit, FVector& OutLocation, FRotator& OutRotation);

	/**
	 * 후보 저장 후 `Magic_IsCharacterRealRiding`(탑승) 이면 바로 PlayAction(0),
	 * 아니면 캐릭터 좌표→후보 로 Yaw 만 맞춘 뒤 PlayAction(0).
	 */
	UFUNCTION(
		BlueprintCallable,
		Category = "Magic|Warp|Utility",
		meta = (DisplayName = "Set Candidate"))
	void SetCandidate(const FVector& InLocation);

private:
	void Warp_ApplyVisualTemplates();

	APlayerController* Warp_ResolvePlayerController() const;

	/** 레거시 Equip: Decal·Particle 표시 및 FixCamera 등 — HiddenInGame / Visibility 동기화 포함. */
	void Warp_SetPreviewFxVisible(bool bVisible);

	/** Unequip: 상호작용 대상 또는 캐릭터 하나에만 UnFix(블루프린트 IsValid 브랜치와 동일). */
	void Warp_UnFixCameraMatchLegacyUnequipBranch();

	/** `Begin_DoAction` 지면 유효 검사용 하향 트레이스 높이(블루프린트 기본값 800). */
	static constexpr double WarpTerrainProbeDownUU = 800.0;

	/** 하향 미스 후 상향 보정 트레이스(블루프린트 기본값 1000). */
	static constexpr double WarpTerrainProbeUpUU = 1000.0;

	/** 후보 높이 = Location.Z + 반구 높이 + 10cm (블루프린트 Add_DoubleDouble 3항). */
	static constexpr double WarpElevateAboveCandidateZUU = 10.0;

	/** 레거시 `EQ_Location` 블랙보드 벡터. */
	static const FName WarpBlackboardEqLocationKey;

	void Warp_MakeElevatedWarpPoint(FVector& OutPoint) const;
	bool Warp_LineTraceWarpAxis(
		UWorld* World,
		const FVector& StartWorld,
		const FVector& EndWorld,
		FHitResult& OutHit) const;

	void Warp_TryResolveAiWarpLocation(FVector& OutLocation, bool& bOutOk) const;
	void Warp_EndActionAndUnequipIfBusy();
};
