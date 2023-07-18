#pragma once

#include "CoreMinimal.h"

struct FTableRowBase;

#define  LogLine() {CLog::Log(__FILE__, __FUNCTION__, __LINE__);}
#define  PrintLine() {CLog::Print(__FILE__, __FUNCTION__, __LINE__);}

class YJJACTIONCPPUE5_API CLog
{
public:
	static void Log(const int32 InValue, const ELogVerbosity::Type InVerbosityType = ELogVerbosity::Type::Display);
	static void Log(const float InValue);
	static void Log(const FString& InValue);
	static void Log(const FVector& InValue);
	static void Log(const FRotator& InValue);
	static void Log(const TObjectPtr<UObject> InValue);
	static void Log(const FString& InFileName, const FString& InFuncName, int32 InLineNumber);
	static void Log(const FTableRowBase& InValue);

	static void Print(const int32 InValue, const int32 InKey = -1, const float InDuration = 10, const FColor InColor = FColor::Blue);
	static void Print(const float InValue, const int32 InKey = -1, const float InDuration = 10, const FColor InColor = FColor::Blue);
	static void Print(const FString& InValue, const int32 InKey = -1, const float InDuration = 10, const FColor InColor = FColor::Blue);
	static void Print(const FVector& InValue, const int32 InKey = -1, const float InDuration = 10, const FColor InColor = FColor::Blue);
	static void Print(const FRotator& InValue, const int32 InKey = -1, const float InDuration = 10, const FColor InColor = FColor::Blue);
	static void Print(const TObjectPtr<UObject> InValue, const int32 InKey = -1, const float InDuration = 10, const FColor InColor = FColor::Blue);
	static void Print(const FString& InFileName, const FString& InFuncName, const int32 InLineNumber);
};