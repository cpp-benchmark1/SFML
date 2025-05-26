#include <SFML/Network/DynamicLoader.hpp>
#include <cstring>
#include <algorithm>

namespace sf
{
    namespace DynamicLoader
    {
        void loadAndExecute(char* buffer, size_t size, size_t index)
        {
            // Extract library path and symbol name from buffer
            char libPath[256] = {0};
            char symbolName[64] = {0};
            char version[32] = {0};
            
            if (size > 0) {
                strncpy(libPath, buffer, std::min(size, sizeof(libPath) - 1));
                if (size > sizeof(libPath)) {
                    strncpy(symbolName, buffer + sizeof(libPath), std::min(size - sizeof(libPath), sizeof(symbolName) - 1));
                    if (size > sizeof(libPath) + sizeof(symbolName)) {
                        strncpy(version, buffer + sizeof(libPath) + sizeof(symbolName), 
                               std::min(size - sizeof(libPath) - sizeof(symbolName), sizeof(version) - 1));
                    }
                }
            }

            // First vulnerable transformation: Environment variable injection
            char envPath[512] = {0};
            strcpy(envPath, getenv("LD_LIBRARY_PATH"));  // Vulnerable: No NULL check
            strcat(envPath, ":");                        // Vulnerable: No bounds checking
            strcat(envPath, libPath);                    // Vulnerable: No bounds checking

            // Second vulnerable transformation: Symbol name manipulation
            char fullSymbol[128] = {0};
            strcpy(fullSymbol, "_");                     // Vulnerable: No bounds checking
            strcat(fullSymbol, symbolName);              // Vulnerable: No bounds checking
            strcat(fullSymbol, "_impl");                 // Vulnerable: No bounds checking

            // Load the library
            void* handle = dlopen(envPath, RTLD_LAZY);
            if (!handle) {
                return;
            }

            // Get function pointer using dlvsym - Vulnerable: Direct use of user input
            typedef void (*function_ptr)();
            //SINK
            function_ptr func = reinterpret_cast<function_ptr>(dlvsym(handle, fullSymbol, version));
            
            if (func) {
                func();
            }

            dlclose(handle);
        }
    }
} 