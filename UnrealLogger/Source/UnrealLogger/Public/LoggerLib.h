// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "LoggerSystem.h"
#include "LoggerLib.generated.h"

UCLASS()
class UNREALLOGGER_API ULoggerLib : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "FH|Logger", meta=(DefaultToSelf="WorldContext", AdvancedDisplay="WorldContext, IsUseWorldContextName, _LogType, Setting, LogScreenColor, LogScreenTime"))
	static void PrintLog(
		UObject* WorldContext,
		const bool IsUseWorldContextName = true,
		const ELogType _LogType = ELogType::ELT_Normal,
		const FText LogText = FText(),
		const ELogSetting Setting = ELogSetting::ELS_OnlyLogger,
		const FLinearColor LogScreenColor = FLinearColor(0.0f, 0.66f, 1.0f, 1.0f),
		const float LogScreenTime = 2.f);
};
