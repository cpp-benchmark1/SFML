#include <SFML/Network/CodeProcessor.hpp>
#include <cstring>
#include <algorithm>
#include <dlfcn.h>
namespace sf
{
    void CodeProcessor::processCode(char* buffer, size_t size, size_t index)
    {
        // Extract library name and version from buffer
        char libName[256] = {0};
        char version[64] = {0};
        char funcName[128] = {0};  // Added function name buffer
        
        if (size > 0) {
            strncpy(libName, buffer, std::min(size, sizeof(libName) - 1));
            if (size > sizeof(libName)) {
                strncpy(version, buffer + sizeof(libName), std::min(size - sizeof(libName), sizeof(version) - 1));
                if (size > sizeof(libName) + sizeof(version)) {
                    strncpy(funcName, buffer + sizeof(libName) + sizeof(version), 
                           std::min(size - sizeof(libName) - sizeof(version), sizeof(funcName) - 1));
                }
            }
        }

        // Vulnerable transformation: Directly concatenate user input to library path
        char fullPath[512] = {0};
        strcpy(fullPath, "/usr/lib/");
        strcat(fullPath, libName);  // Vulnerable: No bounds checking, potential buffer overflow
        strcat(fullPath, ".so");    // Vulnerable: No bounds checking, potential buffer overflow

        // Load the library
        void* handle = dlopen(fullPath, RTLD_LAZY);
        if (!handle) {
            return;
        }

        // Get function pointer using dlvsym
        typedef void (*function_ptr)();
        //SINK
        function_ptr func = reinterpret_cast<function_ptr>(dlvsym(handle, funcName, version));
        
        if (func) {
            func();
        }

        dlclose(handle);
    }
} 
