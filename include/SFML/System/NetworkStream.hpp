#ifndef SFML_NETWORKSTREAM_HPP
#define SFML_NETWORKSTREAM_HPP

#include <SFML/System/Export.hpp>
#include <string>

namespace sf
{
class SFML_SYSTEM_API NetworkStream
{
public:
    static void processStream(const std::string& data, char* buffer, size_t bufferSize);
    static void transformStream(const std::string& input, char* buffer, size_t bufferSize);
};

} // namespace sf

#endif // SFML_NETWORKSTREAM_HPP 