#pragma once

#include <string>
#include <vector>

namespace elsim::cli {

class CliApp {
   public:
    int run(const std::vector<std::string>& args);

   private:
    int runHelp(const std::vector<std::string>& args);
};

}  // namespace elsim::cli
