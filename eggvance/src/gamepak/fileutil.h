#pragma once

#include <string>
#include <vector>

#include "common/integer.h"

namespace file_util
{
    bool read(const std::string& file, std::vector<u8>& dst);
    bool write(const std::string& file, std::vector<u8>& src);
    bool exists(const std::string& file);
}