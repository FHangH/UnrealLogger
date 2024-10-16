// Fill out your copyright notice in the Description page of Project Settings.


#include "LoggerSystem.h"
#include "LoggerSetting.h"
#include "WebSocketsModule.h"
#include "Kismet/KismetSystemLibrary.h"

bool ULoggerSystem::ShouldCreateSubsystem(UObject* Outer) const
{
	return Super::ShouldCreateSubsystem(Outer);
}

void ULoggerSystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void ULoggerSystem::Deinitialize()
{
	Super::Deinitialize();

	if (LoggerWS.IsValid())
	{
		LoggerWS->Close();
		LoggerWS = nullptr;
	}
}

void ULoggerSystem::MakeLoggerSetting(const FLoggerWebSocketSetting Setting)
{
	const FString WS_URL = FString::Printf(TEXT("ws://%s:%s"), *Setting.WebSocketURL, *Setting.Port);
	FWebSocketsModule& WebSocketModule = FWebSocketsModule::Get();
	
	LoggerWS = WebSocketModule.CreateWebSocket(WS_URL);

	if (LoggerWS.IsValid())
	{
		LoggerWS->OnConnected().AddUObject(this, &ULoggerSystem::OnConnected);
		LoggerWS->OnMessage().AddUObject(this, &ULoggerSystem::OnMessage);
		LoggerWS->OnConnectionError().AddUObject(this, &ULoggerSystem::OnConnectionError);
		LoggerWS->OnClosed().AddUObject(this, &ThisClass::OnClosed);

		LoggerWS->Connect();
	}
}

void ULoggerSystem::OnConnected()
{
	UE_LOG(Logger, Log, TEXT("WebSocket connected!"));
}

void ULoggerSystem::OnMessage(const FString& Message)
{
	UE_LOG(Logger, Log, TEXT("Received message: %s"), *Message);
}

void ULoggerSystem::OnConnectionError(const FString& Error)
{
	UE_LOG(Logger, Error, TEXT("Connection error: %s"), *Error);
}

void ULoggerSystem::OnClosed(int32 StatusCode, const FString& Reason, bool bWasClean)
{
	UE_LOG(Logger, Log, TEXT("WebSocket closed. Status code: %d. Reason: %s. Clean: %s"), StatusCode, *Reason, bWasClean ? TEXT("true") : TEXT("false"));
}

void ULoggerSystem::SendLog(const FLoggerSetting& Setting)
{
	switch (Setting.LogSetting) {
	case ELogSetting::ELS_OnlyLogger:
		PrintUE_Log(Setting, true, false, false);
		break;
	case ELogSetting::ELS_LoggerAndUELog:
		PrintUE_Log(Setting, true, false, true);
		break;
	case ELogSetting::ELS_LoggerAndUEScreen:
		PrintUE_Log(Setting, true, true, false);
		break;
	case ELogSetting::ELS_OnlyUEScreen:
		PrintUE_Log(Setting, false, true, false);
		break;
	case ELogSetting::ELS_OnlyUELog:
		PrintUE_Log(Setting, false, false, true);
		break;
	case ELogSetting::ELS_UEScreenAndUELog:
		PrintUE_Log(Setting, false, true, true);
		break;
	case ELogSetting::ELS_All:
		PrintUE_Log(Setting, true, true, true);
		break;
	case ELogSetting::ELS_Null:
		break;
	}
}

void ULoggerSystem::CloseLogger() const
{
	if (LoggerWS.IsValid())
	{
		LoggerWS->Close();
	}
}

int32 ULoggerSystem::GetLogLevel(const FLoggerSetting& Setting) const
{
	return (Setting.LogType == ELogType::ELT_Normal) ? 0 : (Setting.LogType == ELogType::ELT_Warning) ? 1 : 2;
}

void ULoggerSystem::PrintUE_ConsoleLog(const int32 Level, const FString& Message)
{
	if (Level == 0)
	{
		UE_LOG(Logger, Log, TEXT("%s"), *Message);
	}
	if (Level == 1)
	{
		UE_LOG(Logger, Warning, TEXT("%s"), *Message);
	}
	if (Level == 2)
	{
		UE_LOG(Logger, Error, TEXT("%s"), *Message);
	}
}

void ULoggerSystem::PrintUE_Log(const FLoggerSetting& Setting, const bool IsPrintLogger, const bool IsPrintScreen, const bool IsConsoleLog)
{
	const auto LoggerLevel = GetLogLevel(Setting);
	
	if (IsPrintLogger)
	{
		if (LoggerWS.IsValid() && LoggerWS->IsConnected())
		{
			const auto LoggerString = FString::Printf(TEXT("{\"level\":%d, \"content\":%s}"), LoggerLevel, *Setting.LogText.ToString());
			LoggerWS->Send(LoggerString);
		}
		else
		{
			UE_LOG(Logger, Error, TEXT("WebSocket is not connected!"));
		}
	}
	if (IsConsoleLog)
	{
		PrintUE_ConsoleLog(LoggerLevel, Setting.LogText.ToString());
	}
	if (IsPrintScreen)
	{
		UKismetSystemLibrary::PrintString(GetWorld(), Setting.LogText.ToString(), true, false, Setting.LogScreenColor, Setting.LogScreenTime);
	}
}
