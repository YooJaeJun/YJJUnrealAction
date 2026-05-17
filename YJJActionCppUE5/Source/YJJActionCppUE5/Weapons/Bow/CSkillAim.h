#pragma once

#include "CoreMinimal.h"
#include "Weapons/Bow/FAimData.h"
#include "Weapons/CSkillWeapon.h"
#include "CSkillAim.generated.h"

class USpringArmComponent;
class UCameraComponent;
class AController;
class UUserWidget;
class UCurveFloat;
class USoundBase;

enum class EAimFoVInterpKind : uint8
{
	None = 0,
	ZoomIn = 1,
	ZoomOut = 2,
};

/**
 * 레거시 `/Game/Weapons/Bow/Skill_Aim` — SpringArm/Camera 에 FAimData 반영·Origin 스냅·라인 트레이스 크로스헤어·FoV 타임라인 근사.
 */
UCLASS(Blueprintable)
class YJJACTIONCPPUE5_API ACSkillAim : public ACSkillWeapon
{
	GENERATED_BODY()

public:
	ACSkillAim();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Aim|Data")
	bool Aiming = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Aim|Data")
	FAimData AimData;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Aim|Data")
	FAimData Origin;

	UPROPERTY(BlueprintReadOnly, Category = "Aim")
	TObjectPtr<USpringArmComponent> SpringArm;

	UPROPERTY(BlueprintReadOnly, Category = "Aim")
	TObjectPtr<UCameraComponent> Camera;

	/** 레거시 BP 변수 `Controller` 와 동일 — 트레이스 시 Forward 기준 및 위젯 Owning 플레이어 해석. */
	UPROPERTY(BlueprintReadOnly, Category = "Aim")
	TObjectPtr<AController> Controller;

	UPROPERTY(BlueprintReadOnly, Category = "Aim")
	TObjectPtr<UUserWidget> CrossHair;

	/** 줌 크로스헤어용 Bow_Pull 과 동등한 경로 로드 가능. 설정 없으면 사운드 생략(정상)·로그로만 추적 가능. */
	UPROPERTY(EditDefaultsOnly, Category = "Aim|Sound")
	TObjectPtr<USoundBase> AimBowPullSound;

	/** 줄 보간된 FoV 타깃(월드 디폴트 카메라 FoV 가 아니라 레거시 ZoomIn 결과에 맞춤). */
	UPROPERTY(EditDefaultsOnly, Category = "Aim|FoV")
	float AimZoomTargetFoV = 55.f;

	/** 레거시 Timeline 근사 지속 시간(블프 타임라인 길이에 맞게 에디터에서 조정). */
	UPROPERTY(EditDefaultsOnly, Category = "Aim|FoV")
	float AimZoomFoVInterpSecondsIn = 0.25f;

	UPROPERTY(EditDefaultsOnly, Category = "Aim|FoV")
	float AimZoomFoVInterpSecondsOut = 0.25f;

	/** 비어 있으면 선형 0→1 블랜드(커브는 UCurveFloat 시간 0~1 정규화 권장). */
	UPROPERTY(EditDefaultsOnly, Category = "Aim|FoV")
	TObjectPtr<UCurveFloat> AimZoomFoVCurveIn;

	UPROPERTY(EditDefaultsOnly, Category = "Aim|FoV")
	TObjectPtr<UCurveFloat> AimZoomFoVCurveOut;

	/** BP LineTrace 단일 ObjectTypes — 디폴트는 ObjectTypeQuery4 + Query3 순서 재현. */
	UPROPERTY(EditDefaultsOnly, Category = "Aim|Trace")
	TArray<TEnumAsByte<EObjectTypeQuery>> AimTraceObjectTypes;

	UPROPERTY(EditDefaultsOnly, Category = "Aim|Trace")
	float AimTraceLength = 10000.f;

	UPROPERTY(EditDefaultsOnly, Category = "Aim|Trace")
	FName AimTraceBowSocketName = FName(TEXT("Hand_Bow_Right_Arrow"));

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Aim")
	void Pressed();

	virtual void Pressed_Implementation();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Aim")
	void Released();

	virtual void Released_Implementation();

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Aim|Widget")
	TSubclassOf<UUserWidget> CrossHairWidgetClass;

private:
	void SkillAim_RefreshNeedsTickScratch();
	bool SkillAim_ResolveSpringArmAndCameraScratch();
	void SkillAim_ResolveControllerScratch();
	bool SkillAim_EnsureCrosshairWidgetScratch();
	static void SkillAim_CallCrossHairUiEventIfDefined(UUserWidget* WidgetScratch, const FName& FunctionNameScratch);

	void SkillAim_TryLoadBowPullSoundScratch();

	void SkillAim_OnPressedFlowScratch();
	void SkillAim_OnReleasedFlowScratch();

	void SkillAim_SnapshotOriginFromCurrentScratch();
	void SkillAim_ApplyAimDataToComponentsScratch(const FAimData& DataScratch);
	void SkillAim_RestoreSpringCameraFromOriginScratch();

	void SkillAim_StartFoVInterpScratch(bool bZoomInScratch);
	void SkillAim_TickFoVInterpScratch(float DeltaSeconds);
	float SkillAim_AlphaNormalizedFromInterpScratch(
		float ElapsedScratch,
		float DurationScratch,
		const UCurveFloat* CurveScratch) const;

	void SkillAim_TickCrosshairRayScratch();

	EAimFoVInterpKind FoVInterpScratch = EAimFoVInterpKind::None;
	float FoVInterpElapsedScratch = 0.f;
	float CachedFoVBeforeAimScratch = 70.f;
	/** Released 시점 FoV 에서 시작해 조준 해제 전 상태로 블랜드(짧게 눌렀다 뗐을 때 깜빡임 완화). */
	float CachedFoVAtZoomOutStartScratch = 70.f;

	bool SkillAim_PendingRevealCrossHairAfterZoomInScratch = false;
	bool SkillAim_BowPullSoundPlayedForThisPressScratch = false;
};
