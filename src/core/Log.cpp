#include "core/Log.h"

#include <cstdarg>
#include <cstddef>
#include <cstdio>
#include <ctime>

namespace vlt {
namespace {

const char* levelTag(LogLevel level) {
    switch (level) {
        case LogLevel::Info:
            return "INFO ";
        case LogLevel::Warn:
            return "WARN ";
        case LogLevel::Error:
            return "ERROR";
    }
    return "?????";
}

void currentTimeStamp(char* buffer, std::size_t size) {
    const std::time_t now = std::time(nullptr);
    std::tm local{};

#if defined(_WIN32)
    localtime_s(&local, &now);
#else
    localtime_r(&now, &local);
#endif

    std::strftime(buffer, size, "%H:%M:%S", &local);
}

}  // namespace

void logMessage(LogLevel level, const char* format, ...) {
    char message[2048];
    va_list args;
    va_start(args, format);
    std::vsnprintf(message, sizeof(message), format, args);
    va_end(args);

    char stamp[16];
    currentTimeStamp(stamp, sizeof(stamp));

    std::FILE* stream = (level == LogLevel::Error) ? stderr : stdout;
    std::fprintf(stream, "[%s] [%s] %s\n", stamp, levelTag(level), message);
    std::fflush(stream);
}

}  // namespace vlt
