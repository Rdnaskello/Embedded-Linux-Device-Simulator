#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace elsim::cli::monitor {

struct LedSnapshot {
    std::string name;
    std::uint32_t pin{0};
    bool active_high{true};
    bool is_on{false};
};

struct MonitorSnapshot {
    std::string board_name;

    std::uint32_t gpio_dir{0};
    std::uint32_t gpio_in{0};
    std::uint32_t gpio_out{0};

    std::vector<LedSnapshot> leds;
};

}  // namespace elsim::cli::monitor
