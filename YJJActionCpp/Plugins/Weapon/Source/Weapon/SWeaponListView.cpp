#include "SWeaponListView.h"
#include "Weapons/CWeaponAsset.h"
#include "EngineUtils.h"
#include "Widgets/Input/SSearchBox.h"

void SWeaponTableRow::Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& InOwnerTable)
{
	Data = InArgs._RowData;

	SMultiColumnTableRow<FWeaponRowDataPtr>::Construct(
		FSuperRowType::FArguments().Style(FEditorStyle::Get(), "TableView.DarkRow"), InOwnerTable
	);
}

TSharedRef<SWidget> SWeaponTableRow::GenerateWidgetForColumn(const FName& InColumnName)
{
	FString str;
	if (InColumnName == "Number")
		str = FString::FromInt(Data->Number);
	else if (InColumnName == "Name")
		str = Data->Name;

	return SNew(STextBlock)
		.Text(FText::FromString(str));
}

///////////////////////////////////////////////////////////////////////////////

void SWeaponListView::Construct(const FArguments& InArgs)
{
	OnListViewSelectedItem = InArgs._OnSelectedItem;

	ChildSlot
	[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(2, 0)
		[
			SAssignNew(SearchBox, SSearchBox)
			.SelectAllTextWhenFocused(true)
			.OnTextChanged(this, &SWeaponListView::OnTextChanged)
			.OnTextCommitted(this, &SWeaponListView::OnTextCommitted)
		]
		+ SVerticalBox::Slot()
		.FillHeight(1)	// 1 == 100% 한 줄 다 채우겠다.
		[
			SAssignNew(ListView, SListView<FWeaponRowDataPtr>)
			.HeaderRow
			(
				SNew(SHeaderRow)
				+ SHeaderRow::Column("Number")
				.DefaultLabel(FText::FromString(""))
				.ManualWidth(40)
				+ SHeaderRow::Column("Name")
				.DefaultLabel(FText::FromString("Name"))
			)
			.ListItemsSource(&RowDatas)
			.OnGenerateRow(this, &SWeaponListView::OnGenerateRow)
			.OnSelectionChanged(this, &SWeaponListView::OnSelectionChanged)
			.SelectionMode(ESelectionMode::Single)
		]
		+SVerticalBox::Slot()
		.AutoHeight()
		.VAlign(VAlign_Center)
		.HAlign(HAlign_Right)
		.Padding(FMargin(8, 2))
		[
			SNew(STextBlock)
			.Text(this, &SWeaponListView::OnGetAssetCount)
			// 여기 Text 내용 변경 시, 자동 갱신.
			// Attribute를 함수로 줄 수 있다.
		]
	];

	ReadDataAssetList();
}

void SWeaponListView::SelectDataPtr(const UCWeaponAsset* InAsset)
{
	if (HasRowDataPtr() == false)
		return;

	for (const FWeaponRowDataPtr ptr : RowDatas)
	{
		if (ptr->Asset == InAsset)
		{
			ListView->SetSelection(ptr);

			return;
		}
	}
}

FWeaponRowDataPtr SWeaponListView::GetRowDataPtrByName(FString InAssetName)
{
	for (const FWeaponRowDataPtr ptr : RowDatas)
	{
		if (ptr->Name == InAssetName)
			return ptr;
	}

	return nullptr;
}

FString SWeaponListView::SelectedRowDataPtrName() const
{
	const TArray<FWeaponRowDataPtr> ptrs = ListView->GetSelectedItems();

	if (ptrs.Num() > 0)
		return ptrs[0]->Asset->GetName();

	return "";
}

TSharedRef<ITableRow> SWeaponListView::OnGenerateRow(FWeaponRowDataPtr InRow, const TSharedRef<STableViewBase>& InTable) const
{
	return SNew(SWeaponTableRow, InTable)
		.RowData(InRow);
}

void SWeaponListView::ReadDataAssetList()
{
	RowDatas.Empty();

	TArray<UObject*> objects;
	EngineUtils::FindOrLoadAssetsByPath("/Game/Weapons/", objects, EngineUtils::ATL_Regular);

	int32 index = 0;
	for (UObject* obj : objects)
	{
		const TWeakObjectPtr<UCWeaponAsset> asset = Cast<UCWeaponAsset>(obj);
		if (asset == nullptr)
			continue;

		FString name = asset->GetName();

		if (SearchText.IsEmpty() == false)
			if (name.Contains(SearchText.ToString()) == false)
				continue;

		RowDatas.Add(FWeaponRowData::Make(++index, name, asset.Get()));
	}

	RowDatas.Sort([](const FWeaponRowDataPtr& A, const FWeaponRowDataPtr& B)
	{
		return A->Number < B->Number;
	});

	ListView->RequestListRefresh();
}

FText SWeaponListView::OnGetAssetCount() const
{
	const FString str = FString::Printf(L"%d 에셋", RowDatas.Num());

	return FText::FromString(str);
}

void SWeaponListView::OnTextChanged(const FText& InText)
{
	if (SearchText.CompareToCaseIgnored(InText) == 0)
		return;

	SearchText = InText;
	ReadDataAssetList();
}

void SWeaponListView::OnTextCommitted(const FText& InText, ETextCommit::Type InType)
{
	OnTextChanged(InText);
}

void SWeaponListView::OnSelectionChanged(FWeaponRowDataPtr InDataPtr, ESelectInfo::Type InType) const
{
	if (InDataPtr.IsValid() == false)
		return;

	OnListViewSelectedItem.ExecuteIfBound(InDataPtr);
}
