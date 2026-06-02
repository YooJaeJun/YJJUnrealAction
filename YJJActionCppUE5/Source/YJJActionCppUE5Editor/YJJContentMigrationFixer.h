#pragma once

#include "CoreMinimal.h"
#include "EditorSubsystem.h"
#include "YJJContentMigrationFixer.generated.h"

// 에디터 기동 후 레거시 Content(CDA_*, ParkourData DT) 를 UE5 C++ 타입으로 디스크에 맞춘다.
UCLASS()
class UYJJContentMigrationFixer : public UEditorSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

private:
	void ScheduleStartupMigration(double InDuration);
	void RunStartupMigration();

	static void ResaveWeaponDataAssets();
	static void FixParkourDataTableRowStruct();
	static bool SavePackageForObject(UObject* Object);
};
