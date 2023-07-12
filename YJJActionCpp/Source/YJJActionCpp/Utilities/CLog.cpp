#include "CLog.h"
#include "Engine.h"

DEFINE_LOG_CATEGORY_STATIC(YJJ, Display, All);

void CLog::Log(const int32 InValue, const ELogVerbosity::Type InVerbosityType)
{
	UE_LOG(YJJ, Warning, TEXT("%d"), InValue);
}

void CLog::Log(const float InValue)
{
	UE_LOG(YJJ, Warning, TEXT("%f"), InValue);
}

void CLog::Log(const FString& InValue)
{
	UE_LOG(YJJ, Warning, TEXT("%s"), *InValue);
}

void CLog::Log(const FVector& InValue)
{
	UE_LOG(YJJ, Warning, TEXT("%s"), *InValue.ToString());
}

void CLog::Log(const FRotator& InValue)
{
	UE_LOG(YJJ, Warning, TEXT("%s"), *InValue.ToString());
}

void CLog::Log(const UObject* InValue)
{
	FString str;

	if (IsValid(InValue))
		str.Append(InValue->GetName());

	str.Append(IsValid(InValue) ? " Is Valid" : "Is Not Valid");

	UE_LOG(YJJ, Warning, TEXT("%s"), *str);
}

void CLog::Log(const FString& InFileName, const FString& InFuncName, const int32 InLineNumber)
{
	int32 index = 0, length = 0;
	InFileName.FindLastChar(TEXT('\\'), index);

	length = InFileName.Len() - 1;
	FString fileName = InFileName.Right(length - index);

	UE_LOG(YJJ, Warning, TEXT("%s, %s, %d"), *fileName, *InFuncName, InLineNumber);
}

void CLog::Log(const FTableRowBase& InValue)
{
	UE_LOG(YJJ, Warning, TEXT("%s"), *InValue.StaticStruct()->GetDisplayNameText().ToString());
}

void CLog::Print(const int32 InValue, const int32 InKey, const float InDuration, const FColor InColor)
{
	GEngine->AddOnScreenDebugMessage(InKey, InDuration, InColor, FString::FromInt(InValue));
}

void CLog::Print(const float InValue, const int32 InKey, const float InDuration, const FColor InColor)
{
	GEngine->AddOnScreenDebugMessage(InKey, InDuration, InColor, FString::SanitizeFloat(InValue));
}

void CLog::Print(const FString& InValue, const int32 InKey, const float InDuration, const FColor InColor)
{
	GEngine->AddOnScreenDebugMessage(InKey, InDuration, InColor, InValue);
}

void CLog::Print(const FVector& InValue, const int32 InKey, const float InDuration, const FColor InColor)
{
	GEngine->AddOnScreenDebugMessage(InKey, InDuration, InColor, InValue.ToString());
}

void CLog::Print(const FRotator& InValue, const int32 InKey, const float InDuration, const FColor InColor)
{
	GEngine->AddOnScreenDebugMessage(InKey, InDuration, InColor, InValue.ToString());
}

void CLog::Print(const UObject* InValue, const int32 InKey, const float InDuration, const FColor InColor)
{
	FString str;

	if (IsValid(InValue))
		str.Append(InValue->GetName());

	str.Append(IsValid(InValue) ? " Is Valid" : "Is Not Valid");

	GEngine->AddOnScreenDebugMessage(InKey, InDuration, InColor, str);
}

void CLog::Print(const FString& InFileName, const FString& InFuncName, const int32 InLineNumber)
{
	int32 index = 0, length = 0;
	InFileName.FindLastChar(L'\\', index);

	length = InFileName.Len() - 1;
	FString fileName = InFileName.Right(length - index);

	FString str = FString::Printf(TEXT("%s, %s, %d"), *fileName, *InFuncName, InLineNumber);
	GEngine->AddOnScreenDebugMessage(-1, 10, FColor::Blue, str);
}