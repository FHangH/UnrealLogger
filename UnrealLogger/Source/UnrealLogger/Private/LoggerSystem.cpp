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

void ULoggerSystem::SetGlobalLogSetting(const bool UseGlobalLogSetting, const ELogSetting Global_LogSetting)
{
	IsUseGlobalLogSetting = UseGlobalLogSetting;	
	G_LogSetting = Global_LogSetting;
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

void ULoggerSystem::SendLog(const UObject* WorldContext, const bool IsUseWorldContextName, const FLoggerSetting& Setting)
{
	if (IsUseGlobalLogSetting)
	{
		switch (G_LogSetting)
		{
		case ELogSetting::ELS_OnlyLogger:
			PrintUE_Log(WorldContext, IsUseWorldContextName, Setting, true, false, false);
			break;
		case ELogSetting::ELS_LoggerAndUELog:
			PrintUE_Log(WorldContext, IsUseWorldContextName, Setting, true, false, true);
			break;
		case ELogSetting::ELS_LoggerAndUEScreen:
			PrintUE_Log(WorldContext, IsUseWorldContextName, Setting, true, true, false);
			break;
		case ELogSetting::ELS_OnlyUEScreen:
			PrintUE_Log(WorldContext, IsUseWorldContextName, Setting, false, true, false);
			break;
		case ELogSetting::ELS_OnlyUELog:
			PrintUE_Log(WorldContext, IsUseWorldContextName, Setting, false, false, true);
			break;
		case ELogSetting::ELS_UEScreenAndUELog:
			PrintUE_Log(WorldContext, IsUseWorldContextName, Setting, false, true, true);
			break;
		case ELogSetting::ELS_All:
			PrintUE_Log(WorldContext, IsUseWorldContextName, Setting, true, true, true);
			break;
		case ELogSetting::ELS_Null:
			break;	
		}
	}
	else
	{
		switch (Setting.LogSetting)
		{
		case ELogSetting::ELS_OnlyLogger:
			PrintUE_Log(WorldContext, IsUseWorldContextName, Setting, true, false, false);
			break;
		case ELogSetting::ELS_LoggerAndUELog:
			PrintUE_Log(WorldContext, IsUseWorldContextName, Setting, true, false, true);
			break;
		case ELogSetting::ELS_LoggerAndUEScreen:
			PrintUE_Log(WorldContext, IsUseWorldContextName, Setting, true, true, false);
			break;
		case ELogSetting::ELS_OnlyUEScreen:
			PrintUE_Log(WorldContext, IsUseWorldContextName, Setting, false, true, false);
			break;
		case ELogSetting::ELS_OnlyUELog:
			PrintUE_Log(WorldContext, IsUseWorldContextName, Setting, false, false, true);
			break;
		case ELogSetting::ELS_UEScreenAndUELog:
			PrintUE_Log(WorldContext, IsUseWorldContextName, Setting, false, true, true);
			break;
		case ELogSetting::ELS_All:
			PrintUE_Log(WorldContext, IsUseWorldContextName, Setting, true, true, true);
			break;
		case ELogSetting::ELS_Null:
			break;
		}
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

void ULoggerSystem::PrintUE_Log(const UObject* WorldContext, const bool IsUseWorldContextName, const FLoggerSetting& Setting, const bool IsPrintLogger, const bool IsPrintScreen, const bool IsConsoleLog)
{
	const auto LoggerTime = FString::Printf(TEXT("[%s]"), *FDateTime::Now().ToString());
	const auto LoggerLevel = GetLogLevel(Setting);
	const auto WorldContextObjectName = IsUseWorldContextName ? (WorldContext ? FString::Printf(TEXT("(%s)"), *WorldContext->GetName()) : TEXT("(NULL)")) : FString{""};
	const auto LogContent = FString::Printf(TEXT("%s%s"), *WorldContextObjectName, *Setting.LogText.ToString());

	// 使用 JSON 库构建 LoggerString
	const TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject());
	JsonObject->SetStringField(TEXT("time"), LoggerTime);
	JsonObject->SetNumberField(TEXT("level"), LoggerLevel);
	JsonObject->SetStringField(TEXT("content"), LogContent); // 直接使用 LogContent

	if (IsPrintLogger)
	{
		// 序列化 JSON 对象为字符串
		FString LoggerString;
		const auto Writer = TJsonWriterFactory<>::Create(&LoggerString);
		FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);
		
		if (LoggerWS.IsValid() && LoggerWS->IsConnected())
		{
			LoggerWS->Send(LoggerString);
		}
		else
		{
			UE_LOG(Logger, Error, TEXT("WebSocket is not connected!"));
		}
	}
	if (IsConsoleLog)
	{
		PrintUE_ConsoleLog(LoggerLevel, LogContent);
	}
	if (IsPrintScreen)
	{
		UKismetSystemLibrary::PrintString(WorldContext, LogContent, true, false, Setting.LogScreenColor, Setting.LogScreenTime);
	}
}
