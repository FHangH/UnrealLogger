#pragma once

#include "CoreMinimal.h"
#include "IWebSocket.h"
#include "LoggerSetting.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "LoggerSystem.generated.h"

struct FLoggerSetting;
struct FLoggerWebSocketSetting;

DECLARE_LOG_CATEGORY_EXTERN(Logger, Log, All);
inline DEFINE_LOG_CATEGORY(Logger);

UCLASS()
class UNREALLOGGER_API ULoggerSystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

	/* Property */
protected:
	// Default Setting
	TSharedPtr<IWebSocket> LoggerWS;
	bool IsUseGlobalLogSetting { false };
	ELogType G_LogType { ELogType::ELT_Normal };
	ELogSetting G_LogSetting { ELogSetting::ELS_All };

	// Queue Mode Setting
	bool IsUseQueueMode { false };
	float QueueCheckInterval { 0.1f };
	FTimerHandle QueueTimerHandle;
	TArray<FQueuedLogEntry> LogQueue;
	bool IsTimerStarted { false };

	/* Function */
public:
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	
	UFUNCTION(BlueprintCallable, Category="FH|Logger")
	void MakeLoggerSetting(FLoggerWebSocketSetting Setting);

	UFUNCTION(BlueprintCallable, Category="FH|Logger")
	void SetGlobalLogSetting(const bool UseGlobalLogSetting, ELogSetting Global_LogSetting = ELogSetting::ELS_All);

	UFUNCTION()
	void SendLog(const UObject* WorldContext, const bool IsUseWorldContextName, const FLoggerSetting& Setting);

	UFUNCTION(BlueprintCallable, Category="FH|Logger")
	void CloseLogger();

protected:
	void OnConnected();
	void OnMessage(const FString& Message);
	void OnConnectionError(const FString& Error);
	void OnClosed(int32 StatusCode, const FString& Reason, bool bWasClean);

	void DetermineLogSettings(ELogSetting LogSetting, bool& OutIsPrintLogger, bool& OutIsPrintScreen, bool& OutIsConsoleLog);
	void StartQueueProcessing();
	void ProcessLogQueue();
	
	int32 GetLogLevel(const FLoggerSetting& Setting) const;
	void PrintUE_ConsoleLog(const int32 Level, const FString& Message);
	void PrintUE_Log(const UObject* WorldContext, const bool IsUseWorldContextName, const FLoggerSetting& Setting, const bool IsPrintLogger, const bool IsPrintScreen, const bool IsConsoleLog);
};
