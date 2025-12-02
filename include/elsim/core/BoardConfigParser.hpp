#pragma once

#include <stdexcept>
#include <string>

#include "elsim/core/BoardDescription.hpp"

namespace elsim::core {

// Коди помилок конфігурації плати.
enum class BoardConfigErrorCode {
    MissingField,  // Відсутнє обов'язкове поле
    InvalidType,   // Невірний тип (наприклад, рядок замість числа)
    InvalidValue,  // Невірне значення (негативний розмір, нульова частота тощо)
    DuplicateName,       // Дублікат назви регіону/пристрою
    UnsupportedVersion,  // Непідтримувана версія формату конфігу
};

// Виключення, яке кидає парсер конфігурації плати.
class BoardConfigException : public std::runtime_error {
   public:
    BoardConfigException(BoardConfigErrorCode code, const std::string& message);

    BoardConfigErrorCode code() const noexcept { return code_; }

   private:
    BoardConfigErrorCode code_;
};

// Клас-парсер конфігів board.yaml.
class BoardConfigParser {
   public:
    // Завантажити і розпарсити файл конфігурації плати.
    //
    // Може кинути:
    //  - YAML::ParserException при синтаксичній помилці YAML
    //  - BoardConfigException при семантичній/структурній помилці
    static BoardDescription loadFromFile(const std::string& path);
};

}  // namespace elsim::core
