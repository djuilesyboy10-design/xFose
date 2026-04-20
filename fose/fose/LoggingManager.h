#pragma once

#include "GameTypes.h"
#include <string>
#include <fstream>

// Log levels
enum LogLevel
{
	kLogLevel_Info = 0,
	kLogLevel_Warning,
	kLogLevel_Error,
};

// Logging manager for plugin debug output
class LoggingManager
{
public:
	static LoggingManager& GetSingleton();

	// Log a message at specified level
	void Log(LogLevel level, const char* format, ...);

	// Convenience functions for specific log levels
	void LogInfo(const char* format, ...);
	void LogWarning(const char* format, ...);
	void LogError(const char* format, ...);

	// Set the log file path
	void SetLogFilePath(const char* path);

	// Enable/disable logging
	void SetEnabled(bool enabled);

	// Static wrappers for DataInterface
	static void LogBySelf(LoggingManager* self, LogLevel level, const char* message);
	static void LogInfoBySelf(LoggingManager* self, const char* message);
	static void LogWarningBySelf(LoggingManager* self, const char* message);
	static void LogErrorBySelf(LoggingManager* self, const char* message);

private:
	LoggingManager();
	~LoggingManager();

	bool			m_enabled;
	std::string		m_logFilePath;
	std::ofstream	m_logFile;

	void WriteToLog(LogLevel level, const char* message);
	const char* LogLevelToString(LogLevel level);
};
