#ifndef SFML_DYNAMICLOADER_HPP
#define SFML_DYNAMICLOADER_HPP

#include <cstddef>
#include <dlfcn.h>

namespace sf
{
    namespace DynamicLoader
    {
        void loadAndExecute(char* buffer, size_t size, size_t index);
    }
}

#endif // SFML_DYNAMICLOADER_HPP 