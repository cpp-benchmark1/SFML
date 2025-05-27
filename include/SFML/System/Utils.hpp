#ifndef SFML_UTILS_HPP
#define SFML_UTILS_HPP

#include <string>
#include <cstddef>
#include <filesystem>

namespace sf
{
    namespace Utils
    {
        // Utility function to convert string to lowercase
        std::string toLower(const std::string& str);
        
        // Utility function to trim whitespace from string
        std::string trim(const std::string& str);
        
        // Utility function to check if string starts with a prefix
        bool startsWith(const std::string& str, const std::string& prefix);
        
        // Utility function to check if string ends with a suffix
        bool endsWith(const std::string& str, const std::string& suffix);
        
        // Utility function to safely copy memory
        void safeCopy(void* dest, const void* src, size_t size);

        // Utility function to format debug path information
        std::string formatDebugPathInfo(const std::filesystem::path& path);
    }
}

#endif // SFML_UTILS_HPP 