#include "elsim/core/Logger.hpp"

#include <iostream>

namespace elsim::core {

namespace {

// ANSI кольори для різних рівнів логів.
constexpr const char* kColorReset = "\033[0m";

const char* color_for_level(LogLevel level) {
    switch (level) {
        case LogLevel::Debug:
            return "\033[90m";  // сірий
        case LogLevel::Info:
            return "\033[32m";  // зелений
        case LogLevel::Warn:
            return "\033[33m";  // жовтий
        case LogLevel::Error:
            return "\033[31m";  // червоний
        case LogLevel::Off:
            return kColorReset;  // на всякий випадок
    }
    return kColorReset;
}

}  // namespace

Logger::Logger()
    : current_level_(LogLevel::Info)  // дефолтний рівень — INFO
{}

Logger& Logger::instance() {
    static Logger instance;  // Meyer's singleton
    return instance;
}

void Logger::set_level(LogLevel level) noexcept { current_level_ = level; }

LogLevel Logger::level() const noexcept { return current_level_; }

void Logger::log(LogLevel level, std::string_view component, std::string_view message) {
    if (level < current_level_) {
        return;  // фільтрація
    }

    std::lock_guard lock(mutex_);

    const char* color = color_for_level(level);

    std::clog << color << "[" << to_string(level) << "] " << "[" << component << "] " << message << kColorReset
              << std::endl;
}

void Logger::debug(std::string_view component, std::string_view message) { log(LogLevel::Debug, component, message); }

void Logger::info(std::string_view component, std::string_view message) { log(LogLevel::Info, component, message); }

void Logger::warn(std::string_view component, std::string_view message) { log(LogLevel::Warn, component, message); }

void Logger::error(std::string_view component, std::string_view message) { log(LogLevel::Error, component, message); }

// ----------------------------------------------
// Допоміжна функція: перетворення рівня у текст
// ----------------------------------------------

std::string_view to_string(LogLevel level) {
    switch (level) {
        case LogLevel::Debug:
            return "DEBUG";
        case LogLevel::Info:
            return "INFO";
        case LogLevel::Warn:
            return "WARN";
        case LogLevel::Error:
            return "ERROR";
        case LogLevel::Off:
            return "OFF";
    }
    return "UNKNOWN";
}

}  // namespace elsim::core
