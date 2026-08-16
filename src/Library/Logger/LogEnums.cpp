#include "LogEnums.h"

#include "Library/Serialization/EnumSerialization.h"

// Names come from `logLevelName` so that it stays in sync with what's serialized.
MM_DEFINE_ENUM_SERIALIZATION_FUNCTIONS(LogLevel, CASE_INSENSITIVE, {
    {LOG_NONE, logLevelName(LOG_NONE)},
    {LOG_TRACE, logLevelName(LOG_TRACE)},
    {LOG_DEBUG, logLevelName(LOG_DEBUG)},
    {LOG_INFO, logLevelName(LOG_INFO)},
    {LOG_WARNING, logLevelName(LOG_WARNING)},
    {LOG_ERROR, logLevelName(LOG_ERROR)},
    {LOG_CRITICAL, logLevelName(LOG_CRITICAL)},

    // Compatibility:
    {LOG_TRACE, "verbose"},
})
