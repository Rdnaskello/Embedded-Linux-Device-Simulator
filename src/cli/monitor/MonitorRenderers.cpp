#include "MonitorRenderers.hpp"

#include <iomanip>
#include <sstream>
#include <string_view>

namespace elsim::cli::monitor {

namespace {

std::string Hex32(std::uint32_t v) {
    std::ostringstream oss;
    oss << "0x" << std::hex << std::setw(8) << std::setfill('0') << v << std::dec;
    return oss.str();
}

std::string JsonEscape(std::string_view s) {
    std::string out;
    out.reserve(s.size() + 8);

    for (unsigned char c : s) {
        switch (c) {
            case '\\':
                out += "\\\\";
                break;
            case '"':
                out += "\\\"";
                break;
            case '\b':
                out += "\\b";
                break;
            case '\f':
                out += "\\f";
                break;
            case '\n':
                out += "\\n";
                break;
            case '\r':
                out += "\\r";
                break;
            case '\t':
                out += "\\t";
                break;
            default:
                // Escape control characters as \u00XX
                if (c < 0x20) {
                    std::ostringstream oss;
                    oss << "\\u00" << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(c) << std::dec;
                    out += oss.str();
                } else {
                    out.push_back(static_cast<char>(c));
                }
                break;
        }
    }

    return out;
}

}  // namespace

std::string RenderMonitorText(const MonitorSnapshot& s) {
    std::ostringstream oss;

    // Fixed headers/keys, fixed order, fixed hex layout.
    oss << "BOARD " << s.board_name << "\n";
    oss << "GPIO DIR=" << Hex32(s.gpio_dir) << " IN=" << Hex32(s.gpio_in) << " OUT=" << Hex32(s.gpio_out) << "\n";

    // LEDs: stable order is the order stored in snapshot (which we will keep as YAML order).
    for (const auto& led : s.leds) {
        oss << "LED " << led.name << " pin=" << led.pin << " state=" << (led.is_on ? "ON" : "OFF")
            << " active_high=" << (led.active_high ? 1 : 0) << "\n";
    }

    return oss.str();
}

std::string RenderMonitorJson(const MonitorSnapshot& s) {
    std::ostringstream oss;

    // Minimal stable schema, no optional fields.
    oss << "{\n";
    oss << "  \"board\": {\n";
    oss << "    \"name\": \"" << JsonEscape(s.board_name) << "\"\n";
    oss << "  },\n";
    oss << "  \"gpio\": {\n";
    oss << "    \"dir\": \"" << Hex32(s.gpio_dir) << "\",\n";
    oss << "    \"in\":  \"" << Hex32(s.gpio_in) << "\",\n";
    oss << "    \"out\": \"" << Hex32(s.gpio_out) << "\"\n";
    oss << "  },\n";
    oss << "  \"leds\": [\n";

    for (std::size_t i = 0; i < s.leds.size(); ++i) {
        const auto& led = s.leds[i];
        oss << "    { \"name\": \"" << JsonEscape(led.name) << "\", \"pin\": " << led.pin
            << ", \"active_high\": " << (led.active_high ? "true" : "false") << ", \"state\": \""
            << (led.is_on ? "ON" : "OFF") << "\" }";
        if (i + 1 < s.leds.size()) {
            oss << ",";
        }
        oss << "\n";
    }

    oss << "  ]\n";
    oss << "}\n";

    return oss.str();
}

std::string RenderMonitorNdjson(const MonitorSnapshot& s) {
    std::ostringstream oss;

    // One-line JSON, stable keys, minimal schema (same content as pretty JSON).
    oss << "{" << "\"board\":{\"name\":\"" << JsonEscape(s.board_name) << "\"}," << "\"gpio\":{\"dir\":\""
        << Hex32(s.gpio_dir) << "\"," << "\"in\":\"" << Hex32(s.gpio_in) << "\"," << "\"out\":\"" << Hex32(s.gpio_out)
        << "\"}," << "\"leds\":[";
    for (std::size_t i = 0; i < s.leds.size(); ++i) {
        const auto& led = s.leds[i];
        oss << "{" << "\"name\":\"" << JsonEscape(led.name) << "\"," << "\"pin\":" << led.pin << ","
            << "\"active_high\":" << (led.active_high ? "true" : "false") << "," << "\"state\":\""
            << (led.is_on ? "ON" : "OFF") << "\"" << "}";
        if (i + 1 < s.leds.size()) {
            oss << ",";
        }
    }
    oss << "]}";

    return oss.str();
}

}  // namespace elsim::cli::monitor
