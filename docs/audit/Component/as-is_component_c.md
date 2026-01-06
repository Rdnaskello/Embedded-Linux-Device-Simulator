# Component-C — As-Is State (ELSIM v0.3.0)

## Scope (Evidence Set, Factual)
Компонент-C ідентифікований у `docs/audit_v0_3_0_components_by_target.md` як CMake target `board_config_parser_gpio_led_button_tests`, який включає один файл тестів:
- `tests/test_board_config_parser_gpio_led_button.cpp`

Фактичні артефакти, які беруть участь у роботі цього компонента (у межах наданого scope):
- Код тестів:  
  - `tests/test_board_config_parser_gpio_led_button.cpp` (див. `nl -ba`, рядки 1–46)
- Публічний інтерфейс, який викликається тестами:  
  - `include/elsim/core/BoardConfigParser.hpp`
  - `include/elsim/core/BoardDescription.hpp`
- Реалізація, яку тестують:  
  - `src/core/BoardConfigParser.cpp` (див. `nl -ba`, рядки 1–680)
- YAML приклади, які є input для тестів:  
  - `examples/board-examples/invalid-led-without-gpio.yaml`
  - `examples/board-examples/invalid-pin-out-of-range.yaml`
  - `examples/board-examples/invalid-duplicate-pin.yaml`
- Build/CTest інтеграція (факт підключення тесту як target):  
  - `tests/CMakeLists.txt` (секція `board_config_parser_gpio_led_button_tests`)
  - `CMakeLists.txt` (підключає `tests/` як піддиректорію та вмикає testing)

Інших артефактів (документації/CLI-використання/додаткових тестів), релевантних саме до Component-C, у наданому scope **не виявлено**.

---

## 1. Responsibility (Factual)
Component-C — це набір unit-тестів GoogleTest, який:
- викликає `BoardConfigParser::loadFromFile(path)` для заданих YAML-файлів з `examples/board-examples/`;
- перевіряє, що при певних некоректних конфігураціях кидається `BoardConfigException`;
- перевіряє, що `BoardConfigException::code()` дорівнює очікуваному значенню `BoardConfigErrorCode`.

Факти, підтверджені тестами:
- якщо в board YAML є секція `leds`, але немає секції `gpio`, очікується `BoardConfigErrorCode::MissingField`.  
  Джерело: `tests/test_board_config_parser_gpio_led_button.cpp:19–26`.
- якщо LED має `pin`, який виходить за межі `gpio.pin_count`, очікується `BoardConfigErrorCode::InvalidValue`.  
  Джерело: `tests/test_board_config_parser_gpio_led_button.cpp:28–35`.
- якщо LED і Button використовують один і той самий `pin`, очікується `BoardConfigErrorCode::InvalidValue`.  
  Джерело: `tests/test_board_config_parser_gpio_led_button.cpp:37–44`.

Component-C **не** виконує парсинг/валідацію самостійно; він лише запускає парсер і перевіряє наслідки через механізм тестів (факт виклику `BoardConfigParser::loadFromFile`).  
Джерело: `tests/test_board_config_parser_gpio_led_button.cpp:19–44`.

---

## 2. Public API / External Contract
Зовнішні точки взаємодії, які використовує Component-C (фактичні виклики):
- `elsim::core::BoardConfigParser::loadFromFile(const std::string& path)` — викликається для кожного YAML input.  
  Джерело: `tests/test_board_config_parser_gpio_led_button.cpp:21, 30, 39`.
- `elsim::core::BoardConfigException::code() const noexcept -> BoardConfigErrorCode` — читається код помилки в `catch`.  
  Джерело: `tests/test_board_config_parser_gpio_led_button.cpp:23–25, 32–34, 41–43`.

Input / Output у термінах цього компонента:
- Input: шлях до YAML (рядок), сформований як `ELSIM_SOURCE_DIR / <relative path>`.  
  Джерело: `tests/test_board_config_parser_gpio_led_button.cpp:14–17`.
- Очікуваний observable effect: `BoardConfigParser::loadFromFile` кидає `BoardConfigException`, і `ex.code()` дорівнює очікуваному значенню enum.  
  Джерело: `tests/test_board_config_parser_gpio_led_button.cpp:19–44`.

Build-time external contract:
- Тестовий target отримує `ELSIM_SOURCE_DIR="${CMAKE_SOURCE_DIR}"` як compile definition.  
  Джерело: `tests/CMakeLists.txt` (секція `target_compile_definitions(board_config_parser_gpio_led_button_tests ...)` у наданому файлі).

---

## 3. Internal State Model
Component-C (файл тестів) зберігає лише локальний стан у межах процесу тестування:
- функція `srcPath(rel)` створює повний шлях: `std::filesystem::path(ELSIM_SOURCE_DIR) / rel`, повертає `std::string`.  
  Джерело: `tests/test_board_config_parser_gpio_led_button.cpp:14–17`.
- в кожному тесті внутрішній стан не накопичується між тестами; використовується локальний `try/catch`.  
  Джерело: `tests/test_board_config_parser_gpio_led_button.cpp:19–44`.

Внутрішній стан реалізації, яка тестується (для розуміння observed behavior у межах scope):
- `BoardConfigParser.cpp` формує `BoardDescription` і локально використовує допоміжні структури для валідації:
  - `std::unordered_set<std::string> usedNames` (глобальна унікальність імен пристроїв у межах board).  
    Джерело: `src/core/BoardConfigParser.cpp:587–597`.
  - `std::unordered_set<std::uint32_t> usedPins` (глобальна унікальність pin між LED/Button у межах board).  
    Джерело: `src/core/BoardConfigParser.cpp:628–630`, а також використання при LED/Button парсингу: `437–451`, `507–521`.
- Виключення `BoardConfigException` містить `BoardConfigErrorCode code_`.  
  Джерело: конструктор і збереження коду: `src/core/BoardConfigParser.cpp:17–18`, getter коду — в `include/elsim/core/BoardConfigParser.hpp` (у наданому scope без line numbers).

---

## 4. Runtime Behavior
Формат: Trigger → Preconditions → Steps → Observable effects → Post-state.

### Scenario 4.1 — LedWithoutGpio_ThrowsMissingField
Trigger:
- запуск теста `BoardConfigParser_GpioLedButton.LedWithoutGpio_ThrowsMissingField`.  
  Джерело: `tests/test_board_config_parser_gpio_led_button.cpp:19`.

Preconditions:
- `ELSIM_SOURCE_DIR` визначений під час компіляції тесту.  
  Джерело: `tests/test_board_config_parser_gpio_led_button.cpp:15` (використання), `tests/CMakeLists.txt` (definition).
- YAML input: `examples/board-examples/invalid-led-without-gpio.yaml` містить `leds`, але не містить `gpio`.  
  Джерело YAML: `invalid-led-without-gpio.yaml` (наданий файл).

Steps (реалізація парсера у межах scope):
1) `BoardConfigParser::loadFromFile(path)` викликає `YAML::LoadFile(path)`.  
   Джерело: `src/core/BoardConfigParser.cpp:660–663`.
2) Якщо root містить ключ `board`, береться `root["board"]`.  
   Джерело: `src/core/BoardConfigParser.cpp:667–672`.
3) `parseBoard(boardNode)` визначає `hasLedsSection = bool(root["leds"])`, `hasGpioSection = bool(root["gpio"])`.  
   Джерело: `src/core/BoardConfigParser.cpp:599–604`.
4) Якщо `(hasLedsSection || hasButtonsSection) && !hasGpioSection`, викликається `throwMissingField("gpio")`.  
   Джерело: `src/core/BoardConfigParser.cpp:605–607`, реалізація `throwMissingField`: `30–32`.

Observable effects:
- кидається `BoardConfigException` з `BoardConfigErrorCode::MissingField`.  
  Джерело: `throwMissingField`: `src/core/BoardConfigParser.cpp:30–32`; тест перевіряє `ex.code()`: `tests/test_board_config_parser_gpio_led_button.cpp:23–25`.

Post-state:
- виконання теста завершується без падіння, якщо exception був з потрібним `code`.  
  Джерело: `tests/test_board_config_parser_gpio_led_button.cpp:19–26`.

### Scenario 4.2 — PinOutOfRange_ThrowsInvalidValue
Trigger:
- запуск теста `BoardConfigParser_GpioLedButton.PinOutOfRange_ThrowsInvalidValue`.  
  Джерело: `tests/test_board_config_parser_gpio_led_button.cpp:28`.

Preconditions:
- YAML input має `gpio.pin_count: 2` і LED з `pin: 2`.  
  Джерело YAML: `invalid-pin-out-of-range.yaml` (наданий файл).

Steps:
1) `parseBoard` визначає `pinCount` з `gpio.pin_count` (default 32) і валідує 1..32.  
   Джерело: `src/core/BoardConfigParser.cpp:613–620`.
2) `parseLedsSection(root, pinCount, ...)` читає `pin` і перевіряє `pin >= pinCount`.  
   Джерело: `src/core/BoardConfigParser.cpp:437–447`.
3) При `pin >= pinCount` викликається `throwInvalidValue(..., "must be in range 0..")`.  
   Джерело: `src/core/BoardConfigParser.cpp:445–447`, реалізація `throwInvalidValue`: `39–41`.

Observable effects:
- кидається `BoardConfigException` з `BoardConfigErrorCode::InvalidValue`.  
  Джерело: `throwInvalidValue`: `src/core/BoardConfigParser.cpp:39–41`; тест перевіряє `ex.code()`: `tests/test_board_config_parser_gpio_led_button.cpp:32–34`.

Post-state:
- виконання теста завершується без падіння, якщо exception був з `InvalidValue`.  
  Джерело: `tests/test_board_config_parser_gpio_led_button.cpp:28–35`.

### Scenario 4.3 — DuplicatePin_ThrowsInvalidValue
Trigger:
- запуск теста `BoardConfigParser_GpioLedButton.DuplicatePin_ThrowsInvalidValue`.  
  Джерело: `tests/test_board_config_parser_gpio_led_button.cpp:37`.

Preconditions:
- YAML input має `leds[0].pin: 0` і `buttons[0].pin: 0`.  
  Джерело YAML: `invalid-duplicate-pin.yaml` (наданий файл).

Steps:
1) `parseBoard` створює `usedPins` (резерв 32) і передає його у `parseLedsSection` та `parseButtonsSection`.  
   Джерело: `src/core/BoardConfigParser.cpp:628–630`, виклики: `640–651`.
2) `parseLedsSection` при першому використанні `pin=0` вставляє його в `usedPins`.  
   Джерело: `src/core/BoardConfigParser.cpp:448–450` (перевірка `usedPins.insert(pin).second`).
3) `parseButtonsSection` для `pin=0` повторно намагається вставити pin у `usedPins`; якщо `insert` повертає `false`, кидає `InvalidValue` з причиною "pin is already used...".  
   Джерело: `src/core/BoardConfigParser.cpp:518–520`, реалізація `throwInvalidValue`: `39–41`.

Observable effects:
- кидається `BoardConfigException` з `BoardConfigErrorCode::InvalidValue`.  
  Джерело: `src/core/BoardConfigParser.cpp:39–41`, `518–520`; тест перевіряє `ex.code()`: `tests/test_board_config_parser_gpio_led_button.cpp:41–43`.

Post-state:
- виконання теста завершується без падіння, якщо exception був з `InvalidValue`.  
  Джерело: `tests/test_board_config_parser_gpio_led_button.cpp:37–44`.

---

## 5. Error and Edge Behavior
Поведінка Component-C (тестів) при відхиленнях:
- Якщо `BoardConfigParser::loadFromFile(...)` **не кидає** `BoardConfigException` у тесті, викликається `FAIL() << "Expected BoardConfigException"`, що фіксує невдалий тест.  
  Джерело: `tests/test_board_config_parser_gpio_led_button.cpp:22–23`, `31–32`, `40–41`.
- Якщо кидається `BoardConfigException`, але `ex.code()` **не дорівнює** очікуваному значенню, `EXPECT_EQ` робить тест невдалим.  
  Джерело: `tests/test_board_config_parser_gpio_led_button.cpp:24–25`, `33–34`, `42–43`.
- Якщо кидається **інший тип виключення** (наприклад, `YAML::ParserException` чи інше), у тесті є `catch` лише для `BoardConfigException`, тому інше виключення не перехоплюється і веде до аварійного завершення тестового кейсу (поведінка GoogleTest при неперехопленому exception).  
  Джерело: структура `try { ... } catch (const BoardConfigException& ex) { ... }` у `tests/test_board_config_parser_gpio_led_button.cpp:19–44`; згадка про можливі `YAML::ParserException` у коментарі API: `include/elsim/core/BoardConfigParser.hpp` (наданий файл).

Поведінка реалізації `BoardConfigParser` (у межах наданого scope), яка прямо впливає на observed tests:
- При відсутності обов’язкового поля використовується `throwMissingField(path)` → `BoardConfigErrorCode::MissingField`.  
  Джерело: `src/core/BoardConfigParser.cpp:30–32`.
- При невалідному значенні використовується `throwInvalidValue(path, reason)` → `BoardConfigErrorCode::InvalidValue`.  
  Джерело: `src/core/BoardConfigParser.cpp:39–41`.

Інші edge cases `BoardConfigParser` у цьому файлі існують (наприклад, InvalidType, DuplicateName, UnsupportedVersion, змішування `gpio` section і `devices[].type == 'gpio'`, валідатор `gpio.mmio_base` проти MMIO regions, parsing `active_high` і `mode`), але вони **не перевіряються Component-C** (див. секцію 6).  
Джерела (наявність логіки): `src/core/BoardConfigParser.cpp:34–37`, `43–46`, `360–372`, `374–379`, `523–536`.

---

## 6. Test Coverage (As-Is)
Тести, що належать Component-C (фактично присутні у файлі):
1) `BoardConfigParser_GpioLedButton.LedWithoutGpio_ThrowsMissingField`  
   - Перевіряє: `leds` без `gpio` → `BoardConfigErrorCode::MissingField`.  
   Джерело: `tests/test_board_config_parser_gpio_led_button.cpp:19–26`; реалізація відповідної гілки: `src/core/BoardConfigParser.cpp:599–607`.
2) `BoardConfigParser_GpioLedButton.PinOutOfRange_ThrowsInvalidValue`  
   - Перевіряє: `leds[].pin >= gpio.pin_count` → `BoardConfigErrorCode::InvalidValue`.  
   Джерело: `tests/test_board_config_parser_gpio_led_button.cpp:28–35`; реалізація перевірки: `src/core/BoardConfigParser.cpp:437–447`.
3) `BoardConfigParser_GpioLedButton.DuplicatePin_ThrowsInvalidValue`  
   - Перевіряє: дубль pin між LED і Button → `BoardConfigErrorCode::InvalidValue`.  
   Джерело: `tests/test_board_config_parser_gpio_led_button.cpp:37–44`; реалізація `usedPins` перевірок: `src/core/BoardConfigParser.cpp:448–450`, `518–520`.

Аспекти, для яких **не виявлено тестів у scope Component-C** (факт відсутності у файлі):
- Перевірка `InvalidType` для полів (наприклад, `pin` не число, `leds` не sequence).  
  Логіка існує в `src/core/BoardConfigParser.cpp:34–37`, `405–407`, `475–477`, але тестів у `tests/test_board_config_parser_gpio_led_button.cpp` немає.
- Перевірка `gpio.mmio_base` на наявність MMIO region у `memory[]`.  
  Логіка: `src/core/BoardConfigParser.cpp:365–372`; тестів у scope немає.
- Заборона одночасного використання `gpio` section та `devices[].type == 'gpio'`.  
  Логіка: `src/core/BoardConfigParser.cpp:360–363`; тестів у scope немає.
- Обробка `active_high` для LED/Button (включно з permitted string forms) та помилки boolean parsing.  
  Логіка: `src/core/BoardConfigParser.cpp:71–84`, використання: `453–457`, `523–525`; тестів у scope немає.
- Перевірка `buttons[].mode` (підтримка тільки `momentary`).  
  Логіка: `src/core/BoardConfigParser.cpp:526–536`; тестів у scope немає.
- Перевірка глобальної унікальності імен (`DuplicateName`) між `devices[]`, `gpio0`, leds/buttons.  
  Логіка: `src/core/BoardConfigParser.cpp:587–597`, `387–390`, `432–434`, `502–504`; тестів у scope немає.

---

## 7. Documentation Alignment
Документація/контракти у межах наданого scope:
- `include/elsim/core/BoardConfigParser.hpp` документує, що:
  - `BoardConfigParser::loadFromFile` може кинути `YAML::ParserException` при синтаксичній помилці YAML;
  - може кинути `BoardConfigException` при семантичній/структурній помилці.  
  Джерело: `include/elsim/core/BoardConfigParser.hpp` (коментарі в наданому файлі).

- `include/elsim/core/BoardDescription.hpp` визначає структури результату парсингу (`BoardDescription`, `CpuDescription`, `MemoryRegion`, `DeviceDescription`).  
  Джерело: `include/elsim/core/BoardDescription.hpp` (наданий файл).

Факти про відповідність документації реалізації (у межах scope):
- Наявність `BoardConfigException` і коду помилки підтверджується реалізацією конструктора і helper-throw функцій.  
  Джерело: `src/core/BoardConfigParser.cpp:17–18`, `30–41`.

Поза цими header-коментарями, окрема “користувацька” документація про поля `gpio/leds/buttons` у наданому scope **не виявлена** (README/docs не надані в цьому scope), тому твердження про відповідність/невідповідність до таких документів — **не виявлено у scope**.

---

## 8. Real Usage / Observed Usage
Фактичне використання Component-C у системі збірки/тестів:
- Component-C компілюється як target `board_config_parser_gpio_led_button_tests` і реєструється через `gtest_discover_tests(...)`, тобто запускається через CTest discovery механізм GoogleTest.  
  Джерело: `tests/CMakeLists.txt` (секція `BoardConfigParser GPIO/LED/Button YAML tests (TASK-8.5)` у наданому файлі).

Observed usage під час виконання тестів:
- Component-C викликає `BoardConfigParser::loadFromFile` для трьох YAML-файлів з `examples/board-examples/` і очікує exception-коди.  
  Джерело: `tests/test_board_config_parser_gpio_led_button.cpp:19–44`; YAML inputs — надані файли `invalid-*.yaml`.

Інше використання (CLI, приклади, інші модулі), яке прямо посилається на файл `tests/test_board_config_parser_gpio_led_button.cpp`, у наданому scope **не виявлено**.
