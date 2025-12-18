#pragma once

#include <string>

#include "MonitorSnapshot.hpp"

namespace elsim::cli::monitor {

// Stable “machine-readable text” format (grep-friendly).
std::string RenderMonitorText(const MonitorSnapshot& snapshot);

// Pretty JSON (human-friendly, multi-line).
std::string RenderMonitorJson(const MonitorSnapshot& snapshot);

// NDJSON: single-line JSON object (stream-friendly).
std::string RenderMonitorNdjson(const MonitorSnapshot& snapshot);

}  // namespace elsim::cli::monitor
