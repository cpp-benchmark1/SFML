#ifndef SFML_NETWORKBUFFER_HPP
#define SFML_NETWORKBUFFER_HPP

#include <SFML/System/Export.hpp>
#include <string>

namespace sf
{
class SFML_SYSTEM_API NetworkBuffer
{
public:
    static void processData(const std::string& data, char* buffer, size_t bufferSize);
    static void transformData(const std::string& input, char* buffer, size_t bufferSize);
};

} // namespace sf

#endif // SFML_NETWORKBUFFER_HPP 