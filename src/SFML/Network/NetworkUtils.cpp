#include <SFML/Network/NetworkUtils.hpp>
#include <cstring>

namespace sf
{
namespace NetworkUtils
{
    void processBuffer(char* buffer, size_t size, size_t index)
    {
        // Simple transformation: convert to uppercase
        for (size_t i = 0; i < size; i++)
        {
            if (buffer[i] >= 'a' && buffer[i] <= 'z')
                buffer[i] = buffer[i] - 32;
        }

        
        for (size_t i = 0; i < size; i++)
        {
            //SINK
            buffer[index + i] = 'B';  // Write multiple bytes
        }
    }
}
} 