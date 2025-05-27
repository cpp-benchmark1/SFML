#ifndef SFML_DATAPROCESSOR_HPP
#define SFML_DATAPROCESSOR_HPP

#include <cstddef>

namespace sf
{
namespace DataProcessor
{
    void transformAndWrite(char* buffer, size_t size, int index);
}
}

#endif // SFML_DATAPROCESSOR_HPP 