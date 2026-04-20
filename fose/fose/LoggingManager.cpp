#include "LoggingManager.h"
#include <cstdarg>
#include <ctime>
#include <sstream>

LoggingManager::LoggingManager()
	: m_enabled(true)
	, m_logFilePath("fose_plugin.log")
{
	// Open log file in append mode
	m_logFile.open(m_logFilePath.c_str(), std::ios::app);
	if (m_logFile.is_open())
	{
		m_logFile << "\n=== FOSE Logging Manager Initialized ===\n";
		m_logFile.flush();
	}
}

LoggingManager::~LoggingManager()
{
	if (m_logFile.is_open())
	{
		m_logFile << "\n=== FOSE Logging Manager Shutdown ===\n";
		m_logFile.close();
	}
}

LoggingManager& LoggingManager::GetSingleton()
{
	static LoggingManager instance;
	return instance;
}

void LoggingManager::Log(LogLevel level, const char* format, ...)
{
	if (!m_enabled)
		return;

	va_list args;
	va_start(args, format);

	char buffer[4096];
	vsnprintf(buffer, sizeof(buffer), format, args);
	va_end(args);

	WriteToLog(level, buffer);
}

void LoggingManager::LogInfo(const char* format, ...)
{
	if (!m_enabled)
		return;

	va_list args;
	va_start(args, format);

	char buffer[4096];
	vsnprintf(buffer, sizeof(buffer), format, args);
	va_end(args);

	WriteToLog(kLogLevel_Info, buffer);
}

void LoggingManager::LogWarning(const char* format, ...)
{
	if (!m_enabled)
		return;

	va_list args;
	va_start(args, format);

	char buffer[4096];
	vsnprintf(buffer, sizeof(buffer), format, args);
	va_end(args);

	WriteToLog(kLogLevel_Warning, buffer);
}

void LoggingManager::LogError(const char* format, ...)
{
	if (!m_enabled)
		return;

	va_list args;
	va_start(args, format);

	char buffer[4096];
	vsnprintf(buffer, sizeof(buffer), format, args);
	va_end(args);

	WriteToLog(kLogLevel_Error, buffer);
}

void LoggingManager::SetLogFilePath(const char* path)
{
	if (m_logFile.is_open())
		m_logFile.close();

	m_logFilePath = path;
	m_logFile.open(m_logFilePath.c_str(), std::ios::app);

	if (m_logFile.is_open())
	{
		m_logFile << "\n=== Log file changed to: " << m_logFilePath << " ===\n";
		m_logFile.flush();
	}
}

void LoggingManager::SetEnabled(bool enabled)
{
	m_enabled = enabled;
}

void LoggingManager::WriteToLog(LogLevel level, const char* message)
{
	if (!m_logFile.is_open())
		return;

	// Get current time
	time_t now = time(nullptr);
	char timeStr[64];
	strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", localtime(&now));

	// Write log entry
	m_logFile << "[" << timeStr << "] [" << LogLevelToString(level) << "] " << message << "\n";
	m_logFile.flush();

	// Also output to debug console
	_MESSAGE("[FOSE Log] [%s] %s", LogLevelToString(level), message);
}

const char* LoggingManager::LogLevelToString(LogLevel level)
{
	switch (level)
	{
	case kLogLevel_Info:
		return "INFO";
	case kLogLevel_Warning:
		return "WARNING";
	case kLogLevel_Error:
		return "ERROR";
	default:
		return "UNKNOWN";
	}
}

// Static wrappers for DataInterface
void LoggingManager::LogBySelf(LoggingManager* self, LogLevel level, const char* message)
{
	if (self)
		self->Log(level, "%s", message);
}

void LoggingManager::LogInfoBySelf(LoggingManager* self, const char* message)
{
	if (self)
		self->LogInfo("%s", message);
}

void LoggingManager::LogWarningBySelf(LoggingManager* self, const char* message)
{
	if (self)
		self->LogWarning("%s", message);
}

void LoggingManager::LogErrorBySelf(LoggingManager* self, const char* message)
{
	if (self)
		self->LogError("%s", message);
}
