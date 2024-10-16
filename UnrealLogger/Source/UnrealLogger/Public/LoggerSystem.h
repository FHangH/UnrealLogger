// Fill out your copyright notice in the Description page of Project Settings.

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
	TSharedPtr<IWebSocket> LoggerWS;

	/* Function */
public:
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	
	UFUNCTION(BlueprintCallable, Category="FH|Logger")
	void MakeLoggerSetting(FLoggerWebSocketSetting Setting);

	UFUNCTION()
	void SendLog(const UObject* WorldContext, const bool IsUseWorldContextName, const FLoggerSetting& Setting);

	UFUNCTION(BlueprintCallable, Category="FH|Logger")
	void CloseLogger() const;

protected:
	void OnConnected();
	void OnMessage(const FString& Message);
	void OnConnectionError(const FString& Error);
	void OnClosed(int32 StatusCode, const FString& Reason, bool bWasClean);
	
	int32 GetLogLevel(const FLoggerSetting& Setting) const;
	void PrintUE_ConsoleLog(const int32 Level, const FString& Message);
	void PrintUE_Log(const UObject* WorldContext, const bool IsUseWorldContextName, const FLoggerSetting& Setting, const bool IsPrintLogger, const bool IsPrintScreen, const bool IsConsoleLog);
};
