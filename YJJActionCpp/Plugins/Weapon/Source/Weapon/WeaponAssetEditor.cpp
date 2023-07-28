#include "WeaponAssetEditor.h"
#include "SWeaponListView.h"
#include "SWeaponDetailsView.h"
#include "SEquipmentData.h"
#include "SActData.h"
#include "SHitData.h"
#include "Weapons/CWeaponAsset.h"

const FName FWeaponAssetEditor::EditorName = "WeaponAssetEditor";
const FName FWeaponAssetEditor::LeftAreaTabId = "LeftArea";
const FName FWeaponAssetEditor::DetailTabId = "Details";

TSharedPtr<FWeaponAssetEditor> FWeaponAssetEditor::Instance = nullptr;

void FWeaponAssetEditor::OpenWindow(FString InAssetName)
{
	if (Instance.IsValid())
	{
		// �̹� ������ â�� �����ִ� ���¿��� �ٸ� Data Asset ���� ������,
		// -> ���� �׸����� ���� ������ â ����
		// �� �ڵ� ������ â�� ���� ������ ��
		if (Instance->LeftArea.IsValid())
		{
			FWeaponRowDataPtr ptr = nullptr;

			if (InAssetName.Len() > 0)
				ptr = Instance->LeftArea->GetRowDataPtrByName(InAssetName);

			if (ptr.IsValid() == false)
				ptr = Instance->LeftArea->GetFirstDataPtr();

			Instance->LeftArea->SelectDataPtr(ptr->Asset);

			return;
		}

		Instance->CloseWindow();

		Instance.Reset();
		Instance = nullptr;
	}

	Instance = MakeShareable(new FWeaponAssetEditor());
	Instance->Open(InAssetName);
}

void FWeaponAssetEditor::Shutdown()
{
	if (Instance.IsValid())
	{
		Instance->CloseWindow();

		Instance.Reset();
		Instance = nullptr;
	}
}

void FWeaponAssetEditor::Open(FString InAssetName)
{
	LeftArea = SNew(SWeaponListView)
		.OnSelectedItem(this, &FWeaponAssetEditor::OnListViewSelectedItem);

	FPropertyEditorModule& prop = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");

	// DetailsView
	{
		FDetailsViewArgs args(
			false, 
			false, 
			true, 
			FDetailsViewArgs::HideNameArea);
		args.ViewIdentifier = "WeaponAssetEditorDetailsView";
		DetailsView = prop.CreateDetailView(args);

		FOnGetDetailCustomizationInstance detailView;
		detailView.BindStatic(&SWeaponDetailsView::MakeInstance);
		DetailsView->SetGenericLayoutDetailsDelegate(detailView);
	}

	// EquipmentData
	{
		FOnGetPropertyTypeCustomizationInstance instance;
		instance.BindStatic(&SEquipmentData::MakeInstance);
		prop.RegisterCustomPropertyTypeLayout("EquipmentData", instance);
	}

	// ActData
	{
		FOnGetPropertyTypeCustomizationInstance instance;
		instance.BindStatic(&SActData::MakeInstance);
		prop.RegisterCustomPropertyTypeLayout("ActData", instance);
	}

	// HitData
	{
		FOnGetPropertyTypeCustomizationInstance instance;
		instance.BindStatic(&SHitData::MakeInstance);
		prop.RegisterCustomPropertyTypeLayout("HitData", instance);
	}

	const TSharedRef<FTabManager::FLayout> layout = FTabManager::NewLayout("WeaponAssetEditor_Layout")
	->AddArea
	(
		// ToolBar ����
		FTabManager::NewPrimaryArea()->SetOrientation(Orient_Vertical)
		->Split
		(
			FTabManager::NewStack()
			->SetSizeCoefficient(0.1f)
			->AddTab(GetToolbarTabId(), ETabState::OpenedTab)
		)
		// Tab ����
		->Split
		(
			FTabManager::NewSplitter()->SetOrientation(Orient_Horizontal)
			->Split
			(
				FTabManager::NewStack()
				->SetSizeCoefficient(0.175f)
				->AddTab(LeftAreaTabId, ETabState::OpenedTab)
				->SetHideTabWell(true)	// ���� ������ ���Ѵٴ� ��
			)
			->Split
			(
				FTabManager::NewStack()
				->SetSizeCoefficient(0.725f)
				->AddTab(DetailTabId, ETabState::OpenedTab)
				->SetHideTabWell(true)
			)
		)
	);

	TWeakObjectPtr<UCWeaponAsset> asset = nullptr;

	// Data Asset ���� ���� ���� -> InAssetName = ���ϸ�
	// ToolBar ��ư���� ���� -> InAssetName = ��

	if (InAssetName.Len() > 0)
	{
		const FWeaponRowDataPtr ptr = LeftArea->GetRowDataPtrByName(InAssetName);

		if (LeftArea->SelectedRowDataPtrName() == InAssetName)
			return;

		if (ptr.IsValid())
			asset = ptr->Asset;
	}

	// ������ ������ �� �Ѿ�� ��� ù ��° �� ����
	if (asset == nullptr)
		asset = LeftArea->GetFirstDataPtr()->Asset;

	FAssetEditorToolkit::InitAssetEditor(
		EToolkitMode::Standalone, 
		TSharedPtr<IToolkitHost>(), 
		EditorName, 
		layout, 
		true, 
		true, 
		asset.Get());

	// DetailsView->SetObject(asset);

	// ��� DetailView�� asset��ü�� ���õȴ�.
	// â�� ���������� ���� ������ ���������� �ϳ��� �����Ѵ�.
	// �׷��� â�� ����� �� DetailsView�� ���� �� �Ǿ� ������ ������.

	LeftArea->SelectDataPtr(asset.Get());
}

bool FWeaponAssetEditor::OnRequestClose()
{
	if (DetailsView.IsValid())
	{
		if (IsValid(GEditor) && 
			IsValid(GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()))
			GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->NotifyAssetClosed(GetEditingObject(), this);

		if (FModuleManager::Get().IsModuleLoaded("PropertyEditor"))
		{
			FPropertyEditorModule& prop = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
			prop.UnregisterCustomPropertyTypeLayout("EquipmentData");
			prop.UnregisterCustomPropertyTypeLayout("ActData");
			prop.UnregisterCustomPropertyTypeLayout("HitData");
		}
	}

	if (LeftArea.IsValid())
		LeftArea.Reset();

	if (DetailsView.IsValid())
		DetailsView.Reset();

	return true;
}

void FWeaponAssetEditor::RegisterTabSpawners(const TSharedRef<FTabManager>& InTabManager)
{
	FAssetEditorToolkit::RegisterTabSpawners(InTabManager);

	FOnSpawnTab leftAreaTab;
	leftAreaTab.BindSP(this, &FWeaponAssetEditor::Spawn_LeftAreaTab);
	TabManager->RegisterTabSpawner(LeftAreaTabId, leftAreaTab);

	FOnSpawnTab detailTab;
	detailTab.BindSP(this, &FWeaponAssetEditor::Spawn_DetailsViewTab);
	TabManager->RegisterTabSpawner(DetailTabId, detailTab);
}

TSharedRef<SDockTab> FWeaponAssetEditor::Spawn_LeftAreaTab(const FSpawnTabArgs& InArgs) const
{
	return SNew(SDockTab)
	[
		LeftArea.ToSharedRef()
	];
}

TSharedRef<SDockTab> FWeaponAssetEditor::Spawn_DetailsViewTab(const FSpawnTabArgs& InArgs) const
{
	return SNew(SDockTab)
	[
		DetailsView.ToSharedRef()
	];
}

void FWeaponAssetEditor::OnListViewSelectedItem(FWeaponRowDataPtr InDataPtr)
{
	if (InDataPtr == nullptr)
		return;

	if (IsValid(GetEditingObject()))
		RemoveEditingObject(GetEditingObject());

	AddEditingObject(InDataPtr->Asset);
	DetailsView->SetObject(InDataPtr->Asset);
}

FName FWeaponAssetEditor::GetToolkitFName() const
{
	return EditorName;
}

FText FWeaponAssetEditor::GetBaseToolkitName() const
{
	return FText::FromName(EditorName);
}

FString FWeaponAssetEditor::GetWorldCentricTabPrefix() const
{
	return EditorName.ToString();
}

FLinearColor FWeaponAssetEditor::GetWorldCentricTabColorScale() const
{
	return FLinearColor(0, 0, 1);
}

FReply FWeaponAssetEditor::OnClicked()
{
	return FReply::Handled();
}
