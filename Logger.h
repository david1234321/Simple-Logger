#pragma once

#define MAX_LOG_MESSAGE_LENGTH 512
#define MAX_FORMAT_STRING_LENGTH 128
#define MAX_TIME_FORMAT_LENGTH 64
#define MAX_LOG_NAME_LENGTH 64

// Predefined logging macros
#define LOG(LEVEL, MESSAGE, ...) (GlobalLogger)->Log(LEVEL, __func__, MESSAGE, __VA_ARGS__)
#define LOG_ERROR(MESSAGE, ...) (GlobalLogger)->Log(LOG_LEVEL_ERROR, __func__, MESSAGE, __VA_ARGS__)
#define LOG_WARNING(MESSAGE, ...) (GlobalLogger)->Log(LOG_LEVEL_WARNING, __func__, MESSAGE, __VA_ARGS__)
#define LOG_INFORMATION(MESSAGE, ...) (GlobalLogger)->Log(LOG_LEVEL_INFORMATION, __func__, MESSAGE, __VA_ARGS__)
#define LOG_VERBOSE(MESSAGE, ...) (GlobalLogger)->Log(LOG_LEVEL_VERBOSE, __func__, MESSAGE, __VA_ARGS__)

// Log control macros
#define LOG_GLOBAL_INIT() (GlobalLogger = new Logger())
#define LOG_GLOBAL_STATUS() (GlobalLogger == NULL ? false : GlobalLogger->GetLogObjectStatus())
#define LOG_GLOBAL_SELF_TEST_VERBOSE() (GlobalLogger->PerformSelfDiagnostics(true))
#define LOG_GLOBAL_SELF_TEST() (GlobalLogger->PerformSelfDiagnostics(false))
#define LOG_GLOBAL_CLEANUP() (delete GlobalLogger, GlobalLogger = NULL)
#define LOG_GLOBAL_CONFIGURE(CONSOLE_LEVEL, FILE_LEVEL, MIRRORED_LOGS)

// Relative or absolute directory where the logs should be stored
#define LOG_DIRECTORY ".\\Logs"
#define LOG_DIRECTORY_WIDE L".\\Logs"

using namespace std;

typedef enum _LOG_LEVEL {

    LOG_LEVEL_NONE = 0x0,
    LOG_LEVEL_ERROR = 0x1,
    LOG_LEVEL_WARNING = 0x2,
    LOG_LEVEL_INFORMATION = 0x4,
    LOG_LEVEL_VERBOSE = 0x8

} LOG_LEVEL;

class Logger
{
public:

    Logger();

    ~Logger();

    void
    Log(
        LOG_LEVEL LogLevel,
        const char* CallingFunction,
        char* LogMessage,
        ...
    );

    bool
    GetLogObjectStatus()
    {
        return m_LogObjectStatus;
    }

    void
    SetCustomLogConfiguration(
        LOG_LEVEL ConsoleLogLevel,
        LOG_LEVEL FileLogLevel,
        unsigned int MirroredLogConfiguration
    );

    void
    PerformSelfDiagnostics(
        bool IsVerbose
    );

private:

    void
    LogInternalProblem(
        char* LogMessage,
        ...
    );

    bool
    IsUniqueLogSpecified(
        LOG_LEVEL LogLevel
    );

    void
    LogTextFile(
        LOG_LEVEL LogLevel,
        char* FormattedLogMessage
    );

    string
    GenerateLogName(
        LOG_LEVEL LogLevel
    );

    const char*
    Logger::LogLevelToString(
        LOG_LEVEL LogLevel
    );

    void
    SetLogObjectStatus(
        bool LogStatus
    )
    {
        m_LogObjectStatus = LogStatus;
    }

    // Current status of the log object
    bool m_LogObjectStatus;

    // Logging filters
    LOG_LEVEL m_ConsoleLogFilterLevel;
    LOG_LEVEL m_TextFileLogFilterLevel;

    // This bitmask causes logs to be mirrored in a custom
    // log file. These logs will be written even if the log
    // doesn't meet the logging filter criteria 
    unsigned int m_MirrorSpecifiedLogs;

    // Log handles
    ofstream* m_DefaultLog;
    ofstream* m_ErrorLog;
    ofstream* m_WarningLog;
    ofstream* m_InformationLog;
    ofstream* m_VerboseLog;

    // Log name
    string m_DefaultLogName;
    string m_ErrorLogName;
    string m_WarningLogName;
    string m_InformationLogName;
    string m_VerboseLogName;
};

extern Logger* GlobalLogger;