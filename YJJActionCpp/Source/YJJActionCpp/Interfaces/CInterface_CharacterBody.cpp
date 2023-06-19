#include "Interfaces/CInterface_CharacterBody.h"
#include "Characters/CCommonCharacter.h"
#include "Components/SkeletalMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"

void ICInterface_CharacterBody::Create_DynamicMaterial(ACCommonCharacter* InCharacter)
{
	uint8 size = InCharacter->GetMesh()->GetMaterials().Num();
	for (int32 i = 0; i < size; i++)
	{
		TWeakObjectPtr<UMaterialInterface> material = InCharacter->GetMesh()->GetMaterials()[i];

		InCharacter->GetMesh()->SetMaterial(i, UMaterialInstanceDynamic::Create(material.Get(), InCharacter));
	}
}

void ICInterface_CharacterBody::ChangeColor(ACCommonCharacter* InCharacter, FLinearColor InColor)
{
	for (UMaterialInterface* material : InCharacter->GetMesh()->GetMaterials())
	{
		TWeakObjectPtr<UMaterialInstanceDynamic> instance = Cast<UMaterialInstanceDynamic>(material);

		if (!!instance.Get())
			instance->SetVectorParameterValue("BodyColor", InColor);
	}
}
