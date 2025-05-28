////////////////////////////////////////////////////////////
//
// SFML - Simple and Fast Multimedia Library
// Copyright (C) 2007-2024 Laurent Gomila (laurent@sfml-dev.org)
//
// This software is provided 'as-is', without any express or implied warranty.
// In no event will the authors be held liable for any damages arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it freely,
// subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented;
//    you must not claim that you wrote the original software.
//    If you use this software in a product, an acknowledgment
//    in the product documentation would be appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such,
//    and must not be misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source distribution.
//
////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/System/Utils.hpp>
#include <algorithm>
#include <cctype>
#include <cstring>
#include <filesystem>

namespace sf
{
    std::string Utils::formatDebugPathInfo(const std::filesystem::path& path)
    {
        std::string result = "Path: " + path.string() + "\n";
        result += "Absolute path: " + std::filesystem::absolute(path).string() + "\n";
        result += "Exists: " + std::string(std::filesystem::exists(path) ? "Yes" : "No") + "\n";
        if (std::filesystem::exists(path))
        {
            result += "Size: " + std::to_string(std::filesystem::file_size(path)) + " bytes\n";
            result += "Is regular file: " + std::string(std::filesystem::is_regular_file(path) ? "Yes" : "No") + "\n";
        }
        return result;
    }

    namespace Utils
    {
        std::string toLower(const std::string& str)
        {
            std::string result = str;
            std::transform(result.begin(), result.end(), result.begin(),
                          [](unsigned char c) { return std::tolower(c); });
            return result;
        }
        
        std::string trim(const std::string& str)
        {
            const std::string whitespace = " \t\r\n";
            size_t start = str.find_first_not_of(whitespace);
            if (start == std::string::npos)
                return "";
            size_t end = str.find_last_not_of(whitespace);
            return str.substr(start, end - start + 1);
        }
        
        bool startsWith(const std::string& str, const std::string& prefix)
        {
            return str.size() >= prefix.size() &&
                   str.compare(0, prefix.size(), prefix) == 0;
        }
        
        bool endsWith(const std::string& str, const std::string& suffix)
        {
            return str.size() >= suffix.size() &&
                   str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
        }
        
        void safeCopy(void* dest, const void* src, size_t size)
        {
            if (dest && src && size > 0)
            {
                std::memcpy(dest, src, size);
            }
        }
    }
}
