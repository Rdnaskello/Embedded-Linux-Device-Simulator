#pragma once

#include <string>
#include <vector>

namespace elsim::cli {

class PressCommand final {
   public:
    void printHelp();
    int execute(const std::vector<std::string>& args);
};

}  // namespace elsim::cli
