#include "LogEnums.h"

#include "Library/Serialization/EnumSerialization.h"

MM_DEFINE_ENUM_SERIALIZATION_FUNCTIONS(LogLevel, CASE_INSENSITIVE, {
    {LOG_NONE, "none"},
    {LOG_TRACE, "trace"},
    {LOG_DEBUG, "debug"},
    {LOG_INFO, "info"},
    {LOG_WARNING, "warning"},
    {LOG_ERROR, "error"},
    {LOG_CRITICAL, "critical"},

    // Compatibility:
    {LOG_TRACE, "verbose"},
})
