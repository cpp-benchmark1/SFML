#ifndef SFML_NETWORKUTILS_HPP

#define SFML_NETWORKUTILS_HPP

#include <cstddef>

namespace sf
{
namespace NetworkUtils
{
    void processBuffer(char* buffer, size_t size, int index);
}
}

#endif // SFML_NETWORKUTILS_HPP 