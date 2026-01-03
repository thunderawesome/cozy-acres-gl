#pragma once
#include <string>

namespace cozy::core::util
{
    class FileSystem
    {
    public:
        static std::string ReadFile(const std::string &filePath);
    };
}