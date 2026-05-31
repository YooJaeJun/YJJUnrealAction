#pragma once

#include "CoreMinimal.h"
#include "EditorSubsystem.h"
#include "YJJWidgetBlueprintFixer.generated.h"

class UWidgetBlueprint;

// 에디터 기동 시 Widget BP Reparent + orphan WidgetVariableNameToGuidMap 정리 + Compile/Save.
// PIE BeginPlay 에서 StaticLoadClass 가 깨진 BP 를 동기 컴파일하며 Ensure 가 터지는 것을 디스크 단계에서 막는다.
UCLASS()
class UYJJWidgetBlueprintFixer : public UEditorSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

private:
	void RunStartupFix();
	void FixAllWidgetBlueprints();

	static UClass* LoadWidgetParentClass(const TCHAR* ClassSuffix);
	static bool ReparentAndSaveWidgetBlueprint(const FSoftObjectPath& AssetPath, UClass* NewParentClass);
	static int32 PruneStaleWidgetVariableGuids(UWidgetBlueprint* WidgetBlueprint);
};
