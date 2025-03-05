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

	CloseLogger();

	if (QueueTimerHandle.IsValid())
	{
		if (const auto World = GetWorld())
		{
			World->GetTimerManager().ClearTimer(QueueTimerHandle);
		}
		QueueTimerHandle.Invalidate();
	}
	LogQueue.Empty();
}

void ULoggerSystem::MakeLoggerSetting(const FLoggerWebSocketSetting Setting)
{
	IsUseQueueMode = Setting.IsUseEnableQueueMode;
	QueueCheckInterval = Setting.QueueCheckIntervalSeconds;

	if (IsUseQueueMode)
	{
		if (const auto World = GetWorld())
		{
			IsTimerStarted = false;
			World->GetTimerManager().SetTimer(QueueTimerHandle, this, &ULoggerSystem::ProcessLogQueue, QueueCheckInterval, true);
			World->GetTimerManager().PauseTimer(QueueTimerHandle);
			UE_LOG(Logger, Log, TEXT("Begin Queue Mode"));
		}
	}
	
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
	LogQueue.Empty();
	UE_LOG(Logger, Log, TEXT("WebSocket closed. Status code: %d. Reason: %s. Clean: %s"), StatusCode, *Reason, bWasClean ? TEXT("true") : TEXT("false"));
}

void ULoggerSystem::DetermineLogSettings(ELogSetting LogSetting, bool& OutIsPrintLogger, bool& OutIsPrintScreen, bool& OutIsConsoleLog)
{
	switch (LogSetting)
	{
	case ELogSetting::ELS_OnlyLogger:
		OutIsPrintLogger = true;
		OutIsPrintScreen = false;
		OutIsConsoleLog = false;
		break;
	case ELogSetting::ELS_LoggerAndUELog:
		OutIsPrintLogger = true;
		OutIsPrintScreen = false;
		OutIsConsoleLog = true;
		break;
	case ELogSetting::ELS_LoggerAndUEScreen:
		OutIsPrintLogger = true;
		OutIsPrintScreen = true;
		OutIsConsoleLog = false;
		break;
	case ELogSetting::ELS_OnlyUEScreen:
		OutIsPrintLogger = false;
		OutIsPrintScreen = true;
		OutIsConsoleLog = false;
		break;
	case ELogSetting::ELS_OnlyUELog:
		OutIsPrintLogger = false;
		OutIsPrintScreen = false;
		OutIsConsoleLog = true;
		break;
	case ELogSetting::ELS_UEScreenAndUELog:
		OutIsPrintLogger = false;
		OutIsPrintScreen = true;
		OutIsConsoleLog = true;
		break;
	case ELogSetting::ELS_All:
		OutIsPrintLogger = true;
		OutIsPrintScreen = true;
		OutIsConsoleLog = true;
		break;
	case ELogSetting::ELS_Null:
	default:
		OutIsPrintLogger = false;
		OutIsPrintScreen = false;
		OutIsConsoleLog = false;
		break;
	}
}

void ULoggerSystem::StartQueueProcessing()
{
	if (const auto World = GetWorld())
	{
		if (QueueTimerHandle.IsValid())
		{
			IsTimerStarted = true;
			World->GetTimerManager().UnPauseTimer(QueueTimerHandle);
		}
		else
		{
			IsTimerStarted = true;
			World->GetTimerManager().SetTimer(QueueTimerHandle, this, &ULoggerSystem::ProcessLogQueue, QueueCheckInterval, true);
		}
		UE_LOG(Logger, Log, TEXT("Start Queue Timer"));
	}
}

void ULoggerSystem::ProcessLogQueue()
{
	if (LogQueue.IsEmpty()) return;

	const auto Entry = LogQueue[0];
	LogQueue.RemoveAt(0);
	PrintUE_Log(Entry.WorldContext, Entry.IsUseWorldContextName, Entry.Setting, Entry.IsPrintLogger, Entry.IsPrintScreen, Entry.IsConsoleLog);

	if (const auto World = GetWorld())
	{
		if (QueueTimerHandle.IsValid() && LogQueue.IsEmpty())
		{
			IsTimerStarted = false;
			World->GetTimerManager().PauseTimer(QueueTimerHandle);
			UE_LOG(Logger, Log, TEXT("Pause Queue Timer"));
		}
	}
}

void ULoggerSystem::SendLog(const UObject* WorldContext, const bool IsUseWorldContextName, const FLoggerSetting& Setting)
{
	bool IsPrintLogger, IsPrintScreen, IsConsoleLog;

	// Determine log settings based on global or per-log config
	if (IsUseGlobalLogSetting)
	{
		DetermineLogSettings(G_LogSetting, IsPrintLogger, IsPrintScreen, IsConsoleLog);
	}
	else
	{
		DetermineLogSettings(Setting.LogSetting, IsPrintLogger, IsPrintScreen, IsConsoleLog);
	}

	if (IsUseQueueMode)
	{
		const FQueuedLogEntry Entry { WorldContext, IsUseWorldContextName, Setting, IsPrintLogger, IsPrintScreen, IsConsoleLog };
		LogQueue.Add(Entry);

		if (!IsTimerStarted)
		{
			StartQueueProcessing();
		}
	}
	else // Immediate processing
	{
		PrintUE_Log(WorldContext, IsUseWorldContextName, Setting, IsPrintLogger, IsPrintScreen, IsConsoleLog);
	}
}

void ULoggerSystem::CloseLogger()
{
	LogQueue.Empty();
	
	if (LoggerWS.IsValid())
	{
		LoggerWS->Close();
		LoggerWS = nullptr;
	}

	if (QueueTimerHandle.IsValid())
	{
		QueueTimerHandle.Invalidate();
	}

	UE_LOG(Logger, Warning, TEXT("Close Logger"));
}

int32 ULoggerSystem::GetLogLevel(const FLoggerSetting& Setting) const
{
	return (Setting.LogType == ELogType::ELT_Normal) ? 0 : (Setting.LogType == ELogType::ELT_Warning) ? 1 : 2;
}

void ULoggerSystem::PrintUE_ConsoleLog(const int32 Level, const FString& Message)
{
	switch (Level)
	{
	case 0: UE_LOG(Logger, Log, TEXT("%s"), *Message); break;
	case 1: UE_LOG(Logger, Warning, TEXT("%s"), *Message); break;
	case 2: UE_LOG(Logger, Error, TEXT("%s"), *Message); break;
	default: ;
	}
}

void ULoggerSystem::PrintUE_Log(const UObject* WorldContext, const bool IsUseWorldContextName, const FLoggerSetting& Setting, const bool IsPrintLogger, const bool IsPrintScreen, const bool IsConsoleLog)
{
	const auto LoggerTime = FString::Printf(TEXT("[%s]"), *FDateTime::Now().ToString());
	const auto LoggerLevel = GetLogLevel(Setting);
	const auto WorldContextObjectName = IsUseWorldContextName ? (WorldContext ? FString::Printf(TEXT("(%s)"), *WorldContext->GetName()) : TEXT("(NULL)")) : FString{""};
	const auto LogContent = FString::Printf(TEXT("%s%s"), *WorldContextObjectName, *Setting.LogText.ToString());

	if (IsPrintLogger && LoggerWS.IsValid() && LoggerWS->IsConnected())
	{
		// 使用 JSON 库构建 LoggerString
		const TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject());
		JsonObject->SetStringField(TEXT("time"), LoggerTime);
		JsonObject->SetNumberField(TEXT("level"), LoggerLevel);
		JsonObject->SetStringField(TEXT("content"), LogContent); // 直接使用 LogContent
		
		// 序列化 JSON 对象为字符串
		FString LoggerString;
		const auto Writer = TJsonWriterFactory<>::Create(&LoggerString);
		FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);
		
		LoggerWS->Send(LoggerString);
	}
	else
	{
		UE_LOG(Logger, Error, TEXT("WebSocket is not connected!"));
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
