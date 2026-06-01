#pragma once
#include <cstdio>
#include <ctime>
#include <string_view>

enum class LogLevel { Debug, Info, Warn, Error };

inline void log(LogLevel level, std::string_view msg) {
    std::time_t t = std::time(nullptr);
    struct tm* tm_info = std::localtime(&t);
    char ts[9];
    std::strftime(ts, sizeof(ts), "%H:%M:%S", tm_info);

    const char* tag;
    switch (level) {
        case LogLevel::Debug: tag = "DEBUG"; break;
        case LogLevel::Info:  tag = "INFO "; break;
        case LogLevel::Warn:  tag = "WARN "; break;
        default:              tag = "ERROR"; break;
    }
    std::fprintf(stderr, "[%s] [%s] %.*s\n", ts, tag,
                 static_cast<int>(msg.size()), msg.data());
}

#ifdef NDEBUG
#define LOG_D(msg) ((void)0)
#else
#define LOG_D(msg) log(LogLevel::Debug, (msg))
#endif
#define LOG_I(msg) log(LogLevel::Info,  (msg))
#define LOG_W(msg) log(LogLevel::Warn,  (msg))
#define LOG_E(msg) log(LogLevel::Error, (msg))
