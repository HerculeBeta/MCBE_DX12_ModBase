#pragma once

#include <format>
#include <source_location>
#include <string>
#include <string_view>
#include <mutex>
#include <fstream>
#include <filesystem>
#include <Windows.h>

enum class LogLevel {
    Trace,
    Debug,
    Info,
    Warn,
    Error,
    Fatal
};

class Logger {
public:
    static Logger& instance();

    void initialize(const std::string& clientName = "MCBE_ModBase", bool enableConsole = true, bool enableFile = true);
    void shutdown();

    void set_level(LogLevel level) { m_minLevel = level; }
    LogLevel get_level() const { return m_minLevel; }

    template <typename... Args>
    void log(LogLevel level, const std::source_location loc, std::format_string<Args...> fmt, Args&&... args) {
        if (level < m_minLevel || !m_initialized) return;

        std::string formatted_msg = std::vformat(fmt.get(), std::make_format_args(args...));
        write_log(level, loc, formatted_msg);
    }

private:
    Logger() = default;
    ~Logger() = default;

    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    void write_log(LogLevel level, const std::source_location& loc, const std::string& message);
    std::filesystem::path get_log_directory() const;

    bool m_initialized = false;
    bool m_consoleAllocated = false;
    bool m_fileOutput = false;
    LogLevel m_minLevel = LogLevel::Info;

    std::ofstream m_logFile;
    std::mutex m_mutex;
};

#define LOG_TRACE(...) ::Logger::instance().log(LogLevel::Trace, std::source_location::current(), __VA_ARGS__)
#define LOG_DEBUG(...) ::Logger::instance().log(LogLevel::Debug, std::source_location::current(), __VA_ARGS__)
#define LOG_INFO(...)  ::Logger::instance().log(LogLevel::Info,  std::source_location::current(), __VA_ARGS__)
#define LOG_WARN(...)  ::Logger::instance().log(LogLevel::Warn,  std::source_location::current(), __VA_ARGS__)
#define LOG_ERROR(...) ::Logger::instance().log(LogLevel::Error, std::source_location::current(), __VA_ARGS__)
#define LOG_FATAL(...) ::Logger::instance().log(LogLevel::Fatal, std::source_location::current(), __VA_ARGS__)