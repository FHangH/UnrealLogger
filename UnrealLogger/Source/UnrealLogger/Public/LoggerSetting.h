#pragma once

#include "LoggerSetting.generated.h"

UENUM(BlueprintType)
enum class ELogType : uint8
{
	ELT_Normal	UMETA(DisplayName="Normal"),
	ELT_Warning	UMETA(DisplayName="Warning"),
	ELT_Error	UMETA(DisplayName="Error")
};

UENUM(BlueprintType)
enum class ELogSetting : uint8
{
	ELS_OnlyLogger			UMETA(DisplayName="Logger"),
	ELS_LoggerAndUELog		UMETA(DisplayName="Logger-UELog"),
	ELS_LoggerAndUEScreen	UMETA(DisplayName="Logger-UEScreen"),
	ELS_OnlyUEScreen		UMETA(DisplayName="UEScreen"),
	ELS_OnlyUELog			UMETA(DisplayName="UELog"),
	ELS_UEScreenAndUELog	UMETA(DisplayName="UEScreen-UELog"),
	ELS_All					UMETA(DisplayName="All"),
	ELS_Null				UMETA(DisplayName="Null")
};

USTRUCT(BlueprintType)
struct FLoggerSetting
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Logger")
	ELogType LogType { ELogType::ELT_Normal };

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Logger")
	FText LogText {};

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Logger")
	ELogSetting LogSetting { ELogSetting::ELS_OnlyLogger };

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Logger")
	FLinearColor LogScreenColor { 0.0f, 0.66f, 1.0f, 1.0f };

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Logger")
	float LogScreenTime { 2.f };
};

USTRUCT(BlueprintType)
struct FLoggerWebSocketSetting
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Logger")
	FString WebSocketURL { "127.0.0.1" };

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Logger")
	FString Port { "6666" };

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Logger")
	bool IsUseEnableQueueMode { false };

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Logger")
	float QueueCheckIntervalSeconds { 0.1f };
};

USTRUCT(BlueprintType)
struct FQueuedLogEntry
{
	GENERATED_BODY()

	UPROPERTY()
	const UObject* WorldContext { nullptr };

	UPROPERTY()
	bool IsUseWorldContextName { false };

	UPROPERTY()
	FLoggerSetting Setting {};

	UPROPERTY()
	bool IsPrintLogger { false };

	UPROPERTY()
	bool IsPrintScreen { false };

	UPROPERTY()
	bool IsConsoleLog { false };
};
