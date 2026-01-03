#include "FileSystem.h"
#include <fstream>
#include <sstream>
#include <iostream>

namespace cozy::core::util
{
    std::string FileSystem::ReadFile(const std::string &filePath)
    {
        std::ifstream file(filePath);
        if (!file.is_open())
        {
            std::cerr << "[FileSystem] Failed to open: " << filePath << std::endl;
            return "";
        }
        std::stringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
    }
}