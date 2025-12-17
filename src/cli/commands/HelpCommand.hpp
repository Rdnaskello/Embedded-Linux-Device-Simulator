#pragma once

#include <string>
#include <vector>

namespace elsim::cli {

class HelpCommand {
   public:
    int execute(const std::vector<std::string>& args);
};

}  // namespace elsim::cli
