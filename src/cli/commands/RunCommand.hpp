#pragma once

#include <string>
#include <vector>

namespace elsim::cli {

class RunCommand {
   public:
    int execute(const std::vector<std::string>& args);

   private:
    static void printHelp();
};

}  // namespace elsim::cli
