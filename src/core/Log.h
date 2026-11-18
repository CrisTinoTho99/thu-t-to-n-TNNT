#pragma once

namespace vlt {

enum class LogLevel {
    Info,
    Warn,
    Error,
};

// Thread-unsafe, printf-style logger. Info/Warn go to stdout, Error to stderr.
// Every message is flushed immediately so console output survives a crash.
void logMessage(LogLevel level, const char* format, ...);

}  // namespace vlt

#define VLT_LOG_INFO(...) ::vlt::logMessage(::vlt::LogLevel::Info, __VA_ARGS__)
#define VLT_LOG_WARN(...) ::vlt::logMessage(::vlt::LogLevel::Warn, __VA_ARGS__)
#define VLT_LOG_ERROR(...) ::vlt::logMessage(::vlt::LogLevel::Error, __VA_ARGS__)
