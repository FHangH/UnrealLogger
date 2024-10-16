// Fill out your copyright notice in the Description page of Project Settings.


#include "LoggerLib.h"
#include "LoggerSystem.h"
#include "Subsystems/SubsystemBlueprintLibrary.h"

void ULoggerLib::PrintLog(UObject* WorldContext, const FLoggerSetting& Setting)
{
	if (const auto GameIns = USubsystemBlueprintLibrary::GetGameInstanceSubsystem(WorldContext, ULoggerSystem::StaticClass()))
	{
		if (const auto LoggerSys = Cast<ULoggerSystem>(GameIns))
		{
			LoggerSys->SendLog(Setting);
		}
	}
	else
	{
		UE_LOG(Logger, Warning, TEXT("Print Log Failed"));
	}
}
