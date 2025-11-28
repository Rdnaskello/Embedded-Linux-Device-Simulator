#pragma once

#include <mutex>
#include <ostream>
#include <string_view>

namespace elsim::core {

enum class LogLevel { Debug = 0, Info = 1, Error = 2, Off = 3 };

class Logger {
   public:
    // Отримати глобальний екземпляр логера (singleton)
    static Logger& instance();

    // Встановити поточний рівень логування
    void set_level(LogLevel level) noexcept;
    LogLevel level() const noexcept;

    // Базовий метод логування
    void log(LogLevel level, std::string_view component, std::string_view message);

    // Зручні обгортки
    void debug(std::string_view component, std::string_view message);
    void info(std::string_view component, std::string_view message);
    void error(std::string_view component, std::string_view message);

   private:
    Logger();
    ~Logger() = default;

    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    LogLevel current_level_;
    std::mutex mutex_;
};

// Допоміжна функція: перетворення LogLevel → текстова мітка
std::string_view to_string(LogLevel level);

}  // namespace elsim::core
