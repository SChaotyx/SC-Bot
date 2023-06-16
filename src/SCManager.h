#pragma once

#include "includes.h"

namespace SCManager {
    void setSCModVariable(std::string, bool);
    void setSCModString(std::string, std::string);
    bool getSCModVariable(std::string);
    std::string getSCModString(std::string);
}