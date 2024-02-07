
#include "Diagnostics/Logging.h"

// -------------------------------------------------------------------------------

static void printTimestamp(Print* _logOutput) {

  // Division constants
  const unsigned long MSECS_PER_SEC       = 1000;
  const unsigned long SECS_PER_MIN        = 60;
  const unsigned long SECS_PER_HOUR       = 3600;
  const unsigned long SECS_PER_DAY        = 86400;

  // Total time
  const unsigned long msecs               =  millis() ;
  const unsigned long secs                =  msecs / MSECS_PER_SEC;

  // Time in components
  const unsigned long componentMilliSeconds         =  msecs % MSECS_PER_SEC;
  const unsigned long componentSeconds             =  secs  % SECS_PER_MIN ;
  const unsigned long componentMinutes             = (secs  / SECS_PER_MIN) % SECS_PER_MIN;
  const unsigned long componentHours               = (secs  % SECS_PER_DAY) / SECS_PER_HOUR;

  // Time as string
  char timestamp[20];
  sprintf(timestamp, "%02d:%02d:%02d.%03d ", (int)componentHours, (int)componentMinutes, (int)componentSeconds, (int)componentMilliSeconds);
  _logOutput->print(timestamp);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static void printLogLevel(Print* _logOutput, int logLevel)
{
    /// ANSI codes for adding colour etc.
    // https://nerdhut.de/2020/04/11/console-application-styling-ansi-escape-codes/
    // However, VS Code terminal does not seem to respect these (i.e. no colour).
    
    switch (logLevel)
    {
        default:
        case 0:_logOutput->print("[SILENT ] " ); break;
        case 1:_logOutput->print("[FATAL  ] "  ); break;
        case 2:_logOutput->print("[ERROR  ] "  ); break;
        case 3:_logOutput->print("[WARNING] "); break;
        case 4:_logOutput->print("[INFO   ] "   ); break;
        case 5:_logOutput->print("[TRACE  ] "  ); break;
        case 6:_logOutput->print("[VERBOSE] "); break;
    }   
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static void printCore(Print* _logOutput)
{
    uint32_t coreNo = xPortGetCoreID();

    char coreStr[5];
    sprintf(coreStr, "[core:%d] ", (int)coreNo);

    _logOutput->print(coreStr);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void printLogSuffix(Print* _logOutput, int logLevel) {
  _logOutput->print("");
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void printLogPrefix(Print* _logOutput, int logLevel) {
    printTimestamp(_logOutput);
    printLogLevel (_logOutput, logLevel);
    printCore(_logOutput);
}

// -------------------------------------------------------------------------------

void initLogging(int logLevel) {
    Log.setPrefix(printLogPrefix); // set prefix similar to NLog
    Log.setSuffix(printLogSuffix); // set suffix 
    Log.begin(logLevel, &Serial);
    Log.setShowLevel(false);    // Do not show loglevel, we will do this in the prefix  // AutoConnectManager::setup();
}

// -------------------------------------------------------------------------------
