// Fill out your copyright notice in the Description page of Project Settings.


#include "LoggerLib.h"
#include "Subsystems/SubsystemBlueprintLibrary.h"

void ULoggerLib::PrintLog(UObject* WorldContext, const bool IsUseWorldContextName, const ELogType _LogType,
	const FText LogText, const ELogSetting Setting, const FLinearColor LogScreenColor, const float LogScreenTime)
{
	if (const auto GameIns = USubsystemBlueprintLibrary::GetGameInstanceSubsystem(WorldContext, ULoggerSystem::StaticClass()))
	{
		if (const auto LoggerSys = Cast<ULoggerSystem>(GameIns))
		{
			const FLoggerSetting LoggerSetting { _LogType, LogText, Setting, LogScreenColor, LogScreenTime };
			LoggerSys->SendLog(WorldContext, IsUseWorldContextName, LoggerSetting);
		}
	}
	else
	{
		UE_LOG(Logger, Warning, TEXT("Print Log Failed"));
	}
}
