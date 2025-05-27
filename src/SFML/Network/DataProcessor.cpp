 #include <SFML/Network/DataProcessor.hpp>
#include <cstring>

namespace sf
{
namespace DataProcessor
{
    void transformAndWrite(char* buffer, size_t size, int index)
    {
        char dest_buffer[10];
        char buf[1] = {'A'};

        // First transformation: Process data format and transform index
        if (size > 2) {
            // Check data format version
            unsigned char formatVersion = buffer[0];
            
            // Transform index based on format version
            if (formatVersion == 1) {
                index = index + 5;  // Offset for version 1
            } else if (formatVersion == 2) {
                index = index - 3;  // Offset for version 2
            }
            
            // Remove format header
            for (size_t i = 0; i < size - 2; i++) {
                buffer[i] = buffer[i + 2];
            }
            size -= 2;
        }

        //SINK
        dest_buffer[index] = buf[0];  // Direct out-of-bounds write with attacker-controlled index
    }
}
} 