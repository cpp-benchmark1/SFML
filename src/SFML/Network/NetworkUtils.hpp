#ifndef SFML_NETWORKUTILS_HPP
#define SFML_NETWORKUTILS_HPP

#include <cstddef>

namespace sf
{
namespace NetworkUtils
{
    void processBuffer(char* buffer, size_t size, size_t index);
}
}

#endif // SFML_NETWORKUTILS_HPP 