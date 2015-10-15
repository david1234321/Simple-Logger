# Simple-Logger
Performs simple macro based logging. Allows for filters to choose file log and console log verbosity. 

## Usage
1. Include Logger.h
2. Somewhere in your program entry point, call LOG_GLOBAL_INIT()
3. <Optionally> Call LOG_GLOBAL_STATUS() to validate that the log was created successfully. This function returns a bool.
4. <Optinoally> Call LOG_GLOBAL_CONFIGURE(CONSOLE_LEVEL, FILE_LEVEL, MIRRORED_LOGS) to set filter levels. Choices are:
  * LOG_LEVEL_NONE
  * LOG_LEVEL_ERROR
  * LOG_LEVEL_WARNING
  * LOG_LEVEL_INFORMATION
  * LOG_LEVEL_VERBOSE
5. Call your trace functions. Choices are:
  * LOG(LEVEL, MESSAGE, ...)
  * LOG_ERROR(MESSAGE, ...)
  * LOG_WARNING(MESSAGE, ...)
  * LOG_INFORMATION(MESSAGE, ...)
  * LOG_VERBOSE(MESSAGE, ...)
6. Call LOG_GLOBAL_CLEANUP() to deallocate all log structures. 

## Example code
    LOG_GLOBAL_INIT();
  
    LOG_WARNING("Testing this %s", "informational message");
    LOG_VERBOSE("This won't show up by default");
    LOG_ERROR("But this will! (As will these fun numbers %d%d%f)", 0, 3, .14159);
  
    LOG_GLOBAL_CLEANUP();
  
## Example output

    [WARNING]: main (Timestamp: 12:07:05 AM) - Testing this informational message
    [ERROR]: main (Timestamp: 12:07:05 AM) - But this will! (As will these fun numbers 030.141590)
