#include "utils/logger.h"
#include <chrono>
#include <iostream>
#include <cstdlib>
#include <unordered_map>

namespace {
    std::string mc_to_ansi(const std::string& input) {
        static const std::unordered_map<char, const char*> colorMap = {
            {'0', "\033[30m"},  // Black
            {'1', "\033[34m"},  // Dark Blue
            {'2', "\033[32m"},  // Dark Green
            {'3', "\033[36m"},  // Dark Aqua
            {'4', "\033[31m"},  // Dark Red
            {'5', "\033[35m"},  // Dark Purple
            {'6', "\033[33m"},  // Gold
            {'7', "\033[37m"},  // Gray
            {'8', "\033[90m"},  // Dark Gray
            {'9', "\033[94m"},  // Blue
            {'a', "\033[92m"},  // Green
            {'b', "\033[96m"},  // Aqua
            {'c', "\033[91m"},  // Red
            {'d', "\033[95m"},  // Light Purple
            {'e', "\033[93m"},  // Yellow
            {'f', "\033[97m"},  // White
            {'r', "\033[0m"},   // Reset
            {'l', "\033[1m"},   // Bold
            {'o', "\033[3m"}    // Italic
        };

        std::string result;
        result.reserve(input.size());

        for (size_t i = 0; i < input.size(); ++i) {
            if ((input[i] == '\xA7' || (static_cast<unsigned char>(input[i]) == 0xC2 && i + 1 < input.size() && static_cast<unsigned char>(input[i + 1]) == 0xA7)) && i + 1 < input.size()) {

                size_t codeIdx = (input[i] == '\xA7') ? i + 1 : i + 2;
                if (codeIdx < input.size()) {
                    char code = static_cast<char>(std::tolower(input[codeIdx]));
                    auto it = colorMap.find(code);
                    if (it != colorMap.end()) {
                        result += it->second;
                        i = codeIdx; // Skip code bytes
                        continue;
                    }
                }
            }
            result += input[i];
        }
        return result;
    }

    std::string strip_mc_colors(const std::string& input) {
        std::string result;
        result.reserve(input.size());

        for (size_t i = 0; i < input.size(); ++i) {
            if (input[i] == '\xA7' && i + 1 < input.size()) {
                i++; // Skip '\xA7' and code
                continue;
            }
            if (static_cast<unsigned char>(input[i]) == 0xC2 && i + 1 < input.size() && static_cast<unsigned char>(input[i + 1]) == 0xA7 && i + 2 < input.size()) {
                i += 2; // Skip UTF-8 '\xC2\xA7' and code
                continue;
            }
            result += input[i];
        }
        return result;
    }

    std::string colorize_status_keywords(std::string text) {
        if (size_t pos = text.find("-> OK"); pos != std::string::npos) {
            text.replace(pos, 5, "-> \033[92mOK\033[0m"); // Green OK
        }
        else if (size_t pos = text.find("-> FAILED"); pos != std::string::npos) {
            text.replace(pos, 9, "-> \033[91mFAILED\033[0m"); // Red FAILED
        }
        return text;
    }
}

Logger& Logger::instance() {
    static Logger logger;
    return logger;
}

std::filesystem::path Logger::get_log_directory() const {
    char* appdata = nullptr;
    size_t len = 0;
    if (_dupenv_s(&appdata, &len, "APPDATA") == 0 && appdata != nullptr) {
        std::filesystem::path path = std::filesystem::path(appdata) / "MCBE_DX12_ModBase" / "logs";
        free(appdata);
        return path;
    }
    return std::filesystem::current_path() / "logs";
}

void Logger::initialize(const std::string& clientName, bool enableConsole, bool enableFile) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_initialized) return;

#if defined(_DEBUG) || !defined(NDEBUG)
    bool showConsole = enableConsole;
#else
    bool showConsole = false;
#endif

    if (showConsole) {
        if (AllocConsole() || AttachConsole(ATTACH_PARENT_PROCESS)) {
            m_consoleAllocated = true;

            FILE* dummy = nullptr;
            freopen_s(&dummy, "CONOUT$", "w", stdout);
            freopen_s(&dummy, "CONIN$", "r", stdin);
            freopen_s(&dummy, "CONOUT$", "w", stderr);

            std::cout.clear();
            std::cerr.clear();
            std::cin.clear();
            std::ios::sync_with_stdio(true);

            HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
            if (hOut != INVALID_HANDLE_VALUE && hOut != nullptr) {
                DWORD dwMode = 0;
                if (GetConsoleMode(hOut, &dwMode)) {
                    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
                    SetConsoleMode(hOut, dwMode);
                }
            }
            SetConsoleTitleA((clientName + " - Debug Console").c_str());
        }
    }

    if (enableFile) {
        try {
            auto logDir = get_log_directory();
            std::filesystem::create_directories(logDir);
            auto logFilePath = logDir / "latest.log";
            m_logFile.open(logFilePath, std::ios::out | std::ios::trunc);
            m_fileOutput = m_logFile.is_open();
        }
        catch (...) {
            m_fileOutput = false;
        }
    }

    m_initialized = true;
}

void Logger::shutdown() {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (!m_initialized) return;

    if (m_logFile.is_open()) {
        m_logFile.flush();
        m_logFile.close();
    }

    if (m_consoleAllocated) {
        HWND consoleWnd = GetConsoleWindow();

        FILE* dummy = nullptr;
        freopen_s(&dummy, "NUL", "w", stdout);
        freopen_s(&dummy, "NUL", "r", stdin);
        freopen_s(&dummy, "NUL", "w", stderr);

        FreeConsole();

        if (consoleWnd != nullptr && IsWindow(consoleWnd)) {
            PostMessageA(consoleWnd, WM_CLOSE, 0, 0);
        }

        m_consoleAllocated = false;
    }

    m_initialized = false;
}

void Logger::write_log(LogLevel level, const std::source_location& loc, const std::string& message) {
    std::lock_guard<std::mutex> lock(m_mutex);

    auto now = std::chrono::system_clock::now();
    auto time_t_now = std::chrono::system_clock::to_time_t(now);
    std::tm time_info{};
    localtime_s(&time_info, &time_t_now);

    std::string time_str = std::format("{:02d}:{:02d}:{:02d}", time_info.tm_hour, time_info.tm_min, time_info.tm_sec);
    std::string filename = std::filesystem::path(loc.file_name()).filename().string();

    const char* level_str = "INFO";
    const char* level_color = "\033[32m"; // Green

    switch (level) {
    case LogLevel::Trace: level_str = "TRACE"; level_color = "\033[90m"; break; // Gray
    case LogLevel::Debug: level_str = "DEBUG"; level_color = "\033[36m"; break; // Cyan
    case LogLevel::Info:  level_str = "INFO";  level_color = "\033[32m"; break; // Green
    case LogLevel::Warn:  level_str = "WARN";  level_color = "\033[33m"; break; // Yellow
    case LogLevel::Error: level_str = "ERROR"; level_color = "\033[31m"; break; // Red
    case LogLevel::Fatal: level_str = "FATAL"; level_color = "\033[35m"; break; // Magenta
    }

    std::string clean_message = strip_mc_colors(message);
    std::string color_message = colorize_status_keywords(mc_to_ansi(message));

    std::string plain_line = std::format("[{}] [{}] [{}:{}] {}\n",
        time_str, level_str, filename, loc.line(), clean_message);

    OutputDebugStringA(plain_line.c_str());

    if (m_fileOutput && m_logFile.is_open()) {
        m_logFile << plain_line;
        m_logFile.flush();
    }

    if (m_consoleAllocated) {
        std::string console_line = std::format("\033[90m[{}]\033[0m {}[{}]\033[0m \033[90m[{}:{}]\033[0m {}\033[0m\n",
            time_str, level_color, level_str, filename, loc.line(), color_message);
        std::cout << console_line;
    }
}