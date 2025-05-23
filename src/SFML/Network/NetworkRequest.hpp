#ifndef SFML_NETWORKREQUEST_HPP
#define SFML_NETWORKREQUEST_HPP

#include <cstddef>

namespace sf
{
    namespace NetworkRequest
    {
        void makeCurlRequest(const char* buffer, size_t size, size_t index);
        void makeUdpRequest(const char* buffer, size_t size, size_t index);
    }
}

#endif // SFML_NETWORKREQUEST_HPP 