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
	UFUNCTION(BlueprintCallable, Category = "FH|Logger", meta=(DefaultToSelf="WorldContext"))
	static void PrintLog(UObject* WorldContext, const bool IsUseWorldContextName = true, const FLoggerSetting& Setting = FLoggerSetting());
};
