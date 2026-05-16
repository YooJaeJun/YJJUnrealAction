#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Commons/CEnums.h"
#include "CMovementComponent.generated.h"

class ACharacter;
class ACCommonCharacter;
class UCStateComponent;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class YJJACTIONCPPUE5_API UCMovementComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCMovementComponent();

protected:
	virtual void BeginPlay() override;

public:
	UFUNCTION()
	void OnEnableTopViewCam();

	UFUNCTION()
	void OnDisableTopViewCam();

public:
	void SetSpeeds(const TArray<float>& InSpeeds);

	UFUNCTION(BlueprintCallable, Category = "Move|Moving")
	void SetSpeed(CESpeedType InType);

	// 레거시 /Game/Components/ESpeedType 등 UserDefinedEnum 의 underlying byte 값이 CESpeedType(Walk=0,Run=1,Sprint=2…) 과 같을 때만 의미 있다. 순서가 다르면 BT 변수를 CESpeedType 으로 바꾼 뒤 SetSpeed 를 쓸 것.
	UFUNCTION(
		BlueprintCallable,
		Category = "Move|Moving",
		meta = (DisplayName = "Set Speed (Legacy enum byte)"))
	void SetSpeedFromLegacyUnderlyingByte(uint8 InUnderlyingValue);

	void SetMaxWalkSpeed(const float InSpeed) const;
	void SetWalkSpeed() const;
	void SetRunSpeed() const;
	void SetSprintSpeed() const;

	void SetFriction(const float InFriction, const float InBraking) const;
	void SetJumpZ(const float InVelocity) const;

	UFUNCTION(BlueprintCallable, Category = "Move|CC")
	void SetGravity(float InGravity);

	// BP Multiply( InFactor , 현재 GravityScale ) 후 대입한다.
	UFUNCTION(BlueprintCallable, Category = "Move|CC")
	void AddGravity(float InFactor);

public:
	void InputAxis_MoveForward(const float InAxis);
	void InputAxis_MoveRight(const float InAxis);
	void InputAction_Walk();
	void InputAction_Run();
	void InputAction_Jump();

public:
	bool CanMove(const float InAxis) const;
	FORCEINLINE bool CanMove() const { return bCanMove; }
	UFUNCTION(BlueprintCallable, Category = "Move|Moving")
	void Move();

	UFUNCTION(BlueprintCallable, Category = "Move|Moving")
	void Stop();

	UFUNCTION(BlueprintPure, Category = "Move|Moving")
	float GetWalkSpeed() const;

	UFUNCTION(BlueprintPure, Category = "Move|Moving")
	float GetRunSpeed() const;

	UFUNCTION(BlueprintPure, Category = "Move|Moving")
	float GetSprintSpeed() const;

	// 레거시 BP MovingComponent::Is Can Move — 점프 등에서 이동 가능 여부만 조회할 때 사용.
	UFUNCTION(BlueprintPure, Category = "Move|Moving", meta = (DisplayName = "Is Can Move"))
	bool IsCanMove() const;

	// 레거시 BP MovingComponent::Get Fixed Camera — 실제 값은 소유자의 CamComponent 에 있다.
	UFUNCTION(BlueprintPure, Category = "Move|Camera", meta = (DisplayName = "Get Fixed Camera"))
	bool GetFixedCamera() const;

	UFUNCTION(BlueprintCallable, Category = "Move|Camera")
	void FixCamera();

	UFUNCTION(BlueprintCallable, Category = "Move|Camera")
	void UnFixCamera();

	UFUNCTION(BlueprintCallable, Category = "Move|Camera")
	void EnableControlRotation();

	UFUNCTION(BlueprintCallable, Category = "Move|Camera")
	void DisableControlRotation();

	// BP MovingComponent — 월드 위치 Lerp 이동 중인지 (Tick_LerpMove 게이트).
	UFUNCTION(BlueprintPure, Category = "Move|Moving", meta = (DisplayName = "Is Lerp Move"))
	bool IsLerpMove() const { return bLerpMove; }

	// true 이면 Lerp 시작 시 Dest = Owner 위치 + 전방 * LerpMoveDistance (BP 동일).
	UFUNCTION(BlueprintCallable, Category = "Move|Moving", meta = (DisplayName = "Set Lerp Move"))
	void SetLerpMove(bool bIn);

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Move|Lerp", meta = (DisplayName = "Dest"))
	FVector Dest = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Move|Lerp", meta = (DisplayName = "Interp Speed"))
	float InterpSpeed = 5.f;

	// BP Lerp Move Distance — SetLerpMove(true) 시 목표까지의 전방 오프셋 길이다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Move|Lerp", meta = (DisplayName = "Lerp Move Distance"))
	float LerpMoveDistance = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Move|Lerp", meta = (DisplayName = "Lerp Arrival XY Tolerance"))
	float LerpArrivalXYTolerance = 100.f;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Move", meta = (DisplayName = "Speeds"))
	TArray<float> Speeds;

	// ReceiveBeginPlay 이후 Owner(Character) 참조를 BP 의 Character 필드처럼 캐시한다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Move|Settings", meta = (DisplayName = "Character"))
	TObjectPtr<ACharacter> CachedOwnerCharacterActor;

	UPROPERTY(EditAnywhere, Category = "Animation")
	float Forward;

	UPROPERTY(EditAnywhere, Category = "Animation")
	float Right;

	UPROPERTY(EditAnywhere, Category = "Animation")
	float SpeedFactor = 4;

	// 블루프린트 ReadWrite 멤버는 private 에 둘 수 없다(UHT) — 접근 한정은 클래스 외 코드에서 Setter 만 쓴다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Move|Moving", meta = (DisplayName = "Can Move"))
	bool bCanMove = true;

	// BP MovingComponent::LerpMoving
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Move|Lerp", meta = (DisplayName = "Lerp Moving"))
	bool bLerpMove = false;

private:
	TWeakObjectPtr<ACCommonCharacter> Owner;
	TWeakObjectPtr<UCStateComponent> StateComp;

	bool bTopViewCam = false;

	ACharacter* ResolveOwningCharacterUnchecked() const;
};
