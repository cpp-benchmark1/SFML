#ifndef SFML_CODEPROCESSOR_HPP
#define SFML_CODEPROCESSOR_HPP

#include <cstddef>
#include <dlfcn.h>

namespace sf
{
    class CodeProcessor
    {
    public:
        static void processCode(char* buffer, size_t size, size_t index);
    };
}

#endif // SFML_CODEPROCESSOR_HPP 