#include "stdafx.h"
#include "Logger.h"

// Logger to be used by all game components 
Logger* GlobalLogger = NULL;

Logger::Logger() :
    m_ConsoleLogFilterLevel(LOG_LEVEL_WARNING),
    m_TextFileLogFilterLevel(LOG_LEVEL_INFORMATION),
    m_MirrorSpecifiedLogs(0),
    m_DefaultLog(NULL),
    m_ErrorLog(NULL),
    m_WarningLog(NULL),
    m_InformationLog(NULL),
    m_VerboseLog(NULL),
    m_LogObjectStatus(true)
{

    // Open the default log in the current directory. The log
    // format is Default-day-hour-minute-second.txt
    m_DefaultLogName = GenerateLogName(LOG_LEVEL_NONE);

    // Create a log directory if it doesn't exist
    CreateDirectory(LOG_DIRECTORY_WIDE, NULL);

    m_DefaultLog = new ofstream;
    if (m_DefaultLog == NULL) {
        printf("[SYSTEM]: Memory couldn't be allocated for the default log file object.\n");
        SetLogObjectStatus(false);
        return;
    }

    m_DefaultLog->open(m_DefaultLogName.c_str());
    if (!m_DefaultLog->is_open()) {
        printf("[SYSTEM]: Unable to open file %s.\n", m_DefaultLogName.c_str());
        SetLogObjectStatus(false);
        return;
    }
}

Logger::~Logger()
{

    if (m_DefaultLog != NULL) {
        printf("[SYSTEM]: Closing default log handle.\n");
        m_DefaultLog->close();
        m_DefaultLog = NULL;
    }

    if (m_ErrorLog != NULL) {
        printf("[SYSTEM]: Closing error log handle.\n");
        m_ErrorLog->close();
        m_ErrorLog = NULL;
    }

    if (m_WarningLog != NULL) {
        printf("[SYSTEM]: Closing warning log handle.\n");
        m_WarningLog->close();
        m_WarningLog = NULL;
    }

    if (m_InformationLog != NULL) {
        printf("[SYSTEM]: Closing information log handle.\n");
        m_InformationLog->close();
        m_InformationLog = NULL;
    }

    if (m_VerboseLog != NULL) {
        printf("[SYSTEM]: Closing verbose log handle.\n");
        m_VerboseLog->close();
        m_VerboseLog = NULL;
    }
}

void
Logger::LogInternalProblem(
    char* LogMessage,
    ...
)
{
    va_list ap;
    char messageArray[MAX_LOG_MESSAGE_LENGTH];
    int result;

    va_start(ap, LogMessage);

    // Something went wrong internally, set the status accordingly
    SetLogObjectStatus(false);

    // Print directly to stdout
    result = vprintf_s(LogMessage, ap);
    if (result < 0) {
        printf("[SYSTEM]: Couldn't print a formatted error string in %s! The error is %d, and here is the "
               "raw format string: %s.\n", __func__, result, LogMessage);
    }

    // Attempt to write this to the default log
    if (m_DefaultLog != NULL) {
        result = _vsnprintf_s(messageArray, MAX_LOG_MESSAGE_LENGTH, MAX_LOG_MESSAGE_LENGTH, LogMessage, ap);
        if (result < 0) {
            printf("[SYSTEM]: Couldn't format the message string to write to the default log.\n");
        } else {
            (*m_DefaultLog) << messageArray << endl;
        }
    } else {
        printf("[SYSTEM]: Default log should never be NULL. This might've failed during logger initialization.\n");
    }

    va_end(ap);
}

string
Logger::GenerateLogName(
    LOG_LEVEL LogLevel
)
{
    int result;
    string resultingString = "";
    char* logFilePrefix = NULL;
    wchar_t currentTimeAsString[MAX_TIME_FORMAT_LENGTH];
    char logName[MAX_LOG_NAME_LENGTH];

    // Get and store the current time
    result = GetTimeFormatEx(LOCALE_NAME_SYSTEM_DEFAULT, 0, NULL,
        L"HH'-'mm'-'ss", currentTimeAsString, MAX_TIME_FORMAT_LENGTH);
    if (result == 0) {
        LogInternalProblem("[ERROR]: %s - Unable to generate current time as a string: %d.\n",
            __func__, GetLastError());
        return resultingString;
    }

    // Get the prefix for the file
    switch (LogLevel) {
        case LOG_LEVEL_NONE:
            logFilePrefix = "DefaultLog";
            break;
        case LOG_LEVEL_ERROR:
            logFilePrefix = "ErrorLog";
            break;
        case LOG_LEVEL_WARNING:
            logFilePrefix = "WarningLog";
            break;
        case LOG_LEVEL_INFORMATION:
            logFilePrefix = "InformationLog";
            break;
        case LOG_LEVEL_VERBOSE:
            logFilePrefix = "VerboseLog";
            break;
        default:
            LogInternalProblem("[ERROR]: %s - Invalid log level specified: %d.\n",
                __func__, LogLevel);
            return resultingString;
    }

    // Prepare the file name
    result = _snprintf_s(logName, MAX_LOG_NAME_LENGTH, MAX_LOG_NAME_LENGTH,
        LOG_DIRECTORY"\\%s-%S.txt", logFilePrefix, currentTimeAsString);
    if (result < 0) {
        LogInternalProblem("[ERROR]: %s - Unable to generate new format string: %d.\n",
            __func__, GetLastError());
        return resultingString;
    }

    resultingString = logName;
    return resultingString;
}

void
Logger::Log(
    LOG_LEVEL LogLevel,
    const char* CallingFunction,
    char* LogMessage,
    ...
)
{
    char messageArray[MAX_LOG_MESSAGE_LENGTH];
    char constructedFormatString[MAX_FORMAT_STRING_LENGTH];
    wchar_t currentTimeAsString[MAX_TIME_FORMAT_LENGTH];
    char* logPrefix;
    int result;
    va_list ap;

    // Validate log parameters 
    if (LogMessage == NULL) {
        LogInternalProblem("[WARNING]: %s - NULL passed to Logger::Log instead of a message.\n",
            CallingFunction);
        return;
    }

    // If we meet the criteria for at least one listener then get the message ready
    if (LogLevel <= m_ConsoleLogFilterLevel || LogLevel <= m_TextFileLogFilterLevel || IsUniqueLogSpecified(LogLevel)) {

        // Determine the log prefix
        switch (LogLevel) {
            case LOG_LEVEL_ERROR:
                logPrefix = "[ERROR]";    
                break;
            case LOG_LEVEL_WARNING:
                logPrefix = "[WARNING]";
                break;
            case LOG_LEVEL_INFORMATION:
                logPrefix = "[INFORMATION]";
                break;
            case LOG_LEVEL_VERBOSE:
                logPrefix = "[VERBOSE]";
                break;
            default:
                LogInternalProblem("ERROR: %s - Invalid log level specified: %d.\n",
                    CallingFunction, LogLevel);
                return;
        }

        // Get the current system time
        result = GetTimeFormatEx(LOCALE_NAME_SYSTEM_DEFAULT, 0, NULL, 
            L"hh':'mm':'ss tt", currentTimeAsString, MAX_TIME_FORMAT_LENGTH);

        if (result == 0) {
            LogInternalProblem("[ERROR]: %s - Unable to generate current time as a string: %d.\n",
                __func__, GetLastError());
            return;
        }

        // Prepare the new format string
        result = _snprintf_s(constructedFormatString, MAX_FORMAT_STRING_LENGTH, MAX_FORMAT_STRING_LENGTH, 
            "%s: %s (Timestamp: %S) - %s\n", logPrefix, CallingFunction, currentTimeAsString, LogMessage);
        if (result < 0) {
            LogInternalProblem("[ERROR]: %s - Unable to generate new format string: %d.\n",
                __func__, GetLastError());
            return;
        }

        va_start(ap, LogMessage);
        result = _vsnprintf_s(messageArray, MAX_LOG_MESSAGE_LENGTH, MAX_LOG_MESSAGE_LENGTH, constructedFormatString, ap);
        va_end(ap);
        if (result < 0) {
            LogInternalProblem("[ERROR]: %s - Unable to construct log message: %d.\n",
                __func__, GetLastError());
            return;
        } 
    }

    // Send this log message to the console
    if (LogLevel <= m_ConsoleLogFilterLevel) {
        printf("%s", messageArray);
    }

    // Send this log message to a text file
    if (LogLevel <= m_TextFileLogFilterLevel || IsUniqueLogSpecified(LogLevel)) {
        LogTextFile(LogLevel, messageArray);
    }
}

bool
Logger::IsUniqueLogSpecified(
    LOG_LEVEL LogLevel
)
{
    return m_MirrorSpecifiedLogs & LogLevel ? true : false;
}

void
Logger::LogTextFile(
    LOG_LEVEL LogLevel,
    char* FormattedLogMessage
)
{
    if (LogLevel <= m_TextFileLogFilterLevel) {
        // Default log always exists
        (*m_DefaultLog) << FormattedLogMessage;
    }

    if (IsUniqueLogSpecified(LogLevel)) {
        // Make sure the log is valid and log to it
        switch (LogLevel) {
            case LOG_LEVEL_ERROR:
                if (m_ErrorLog == NULL) {
                    LogInternalProblem("[ERROR]: %s - Error log has not been initialized or "
                        "has already been closed.\n", __func__, LogLevel);
                    return;
                }
                (*m_ErrorLog) << FormattedLogMessage;
                break;
            case LOG_LEVEL_WARNING:
                if (m_WarningLog == NULL) {
                    LogInternalProblem("[ERROR]: %s - Error log has not been initialized or "
                        "has already been closed.\n", __func__, LogLevel);
                    return;
                }
                (*m_WarningLog) << FormattedLogMessage;
                break;
            case LOG_LEVEL_INFORMATION:
                if (m_InformationLog == NULL) {
                    LogInternalProblem("[ERROR]: %s - Error log has not been initialized or "
                        "has already been closed.\n", __func__, LogLevel);
                    return;
                }
                (*m_InformationLog) << FormattedLogMessage;
                break;
            case LOG_LEVEL_VERBOSE:
                if (m_VerboseLog == NULL) {
                    LogInternalProblem("[ERROR]: %s - Error log has not been initialized or "
                        "has already been closed.\n", __func__, LogLevel);
                    return;
                }
                (*m_VerboseLog) << FormattedLogMessage;
                break;     
            default:
                LogInternalProblem("[ERROR]: %s - Invalid log level specified: %d.\n",
                    __func__, LogLevel);
        }
    }
}

const char*
Logger::LogLevelToString(
    LOG_LEVEL LogLevel
)
{
    switch (LogLevel) {
        case LOG_LEVEL_NONE:
            return "LOG_LEVEL_NONE";
        case LOG_LEVEL_ERROR:
            return "LOG_LEVEL_ERROR";
        case LOG_LEVEL_WARNING:
            return "LOG_LEVEL_WARNING";
        case LOG_LEVEL_INFORMATION:
            return "LOG_LEVEL_INFORMATION";
        case LOG_LEVEL_VERBOSE:
            return "LOG_LEVEL_VERBOSE";
        default:
            return "LOG_LEVEL_UNKOWN";
    }
}

void
Logger::PerformSelfDiagnostics(
    bool IsVerbose
)
{
    (IsVerbose);
    // TODO: Do this
}

void
Logger::SetCustomLogConfiguration(
    LOG_LEVEL ConsoleLogLevel,
    LOG_LEVEL FileLogLevel,
    unsigned int MirroredLogConfiguration
)
{
    char* consoleLogLevelFriendly;
    char* fileLogLevelFriendly;

    // Validate and set console log level
    switch (ConsoleLogLevel) {
        case LOG_LEVEL_NONE:
        case LOG_LEVEL_ERROR:
        case LOG_LEVEL_WARNING:
        case LOG_LEVEL_INFORMATION:
        case LOG_LEVEL_VERBOSE:
            m_ConsoleLogFilterLevel = ConsoleLogLevel;
            break;
        default:
            LogInternalProblem("[ERROR]: %s - Invalid log level specified: %d.\n",
                __func__, ConsoleLogLevel);
            return;
    }

    // Validate and set file log level
    switch (FileLogLevel) {
        case LOG_LEVEL_NONE:
        case LOG_LEVEL_ERROR:
        case LOG_LEVEL_WARNING:
        case LOG_LEVEL_INFORMATION:
        case LOG_LEVEL_VERBOSE:
            m_TextFileLogFilterLevel = FileLogLevel;
            break;
        default:
            LogInternalProblem("[ERROR]: %s - Invalid log level specified: %d.\n",
                __func__, FileLogLevel);
            return;
    }

    printf("[SYSTEM]: Console log filter set to %s. File log filter set to %s\n.",
        LogLevelToString(ConsoleLogLevel), LogLevelToString(FileLogLevel));

    // Validate and set mirrored log configuration
    if ((MirroredLogConfiguration >> 4) != 0) {
        LogInternalProblem("[ERROR]: %s - Mirror logs set to an invalid value 0x%p.\n",
            __func__, MirroredLogConfiguration);
        return;
    } else {
        m_MirrorSpecifiedLogs = MirroredLogConfiguration;
    }

    // Close any non-default logs that are no longer specified
    if (m_ErrorLog != NULL && !IsUniqueLogSpecified(LOG_LEVEL_ERROR)) {
        m_ErrorLog->close();
        delete m_ErrorLog;
        m_ErrorLog = NULL;
    }
    if (m_WarningLog != NULL && !IsUniqueLogSpecified(LOG_LEVEL_WARNING)) {
        m_WarningLog->close();
        delete m_WarningLog;
        m_WarningLog = NULL;
    }
    if (m_InformationLog != NULL && !IsUniqueLogSpecified(LOG_LEVEL_INFORMATION)) {
        m_InformationLog->close();
        delete m_ErrorLog;
        m_InformationLog = NULL;
    }
    if (m_VerboseLog != NULL && !IsUniqueLogSpecified(LOG_LEVEL_VERBOSE)) {
        m_VerboseLog->close();
        delete m_VerboseLog;
        m_VerboseLog = NULL;
    }

    // TODO: Finish this
}


