#include <SFML/Network/NetworkUtils.hpp>
#include <cstring>

namespace sf
{
namespace NetworkUtils
{
    void processBuffer(char* buffer, size_t size, int index)
    {
        char dest_buffer[10];
        char buf[1] = {'B'};

        // First transformation: Process network packet header
        if (size > 4) {
            unsigned char packetType = buffer[0];
            unsigned short dataLength = (buffer[1] << 8) | buffer[2];
            
            // Transform index based on packet type
            if (packetType == 0x01) {
                index = index << 1;  // Double the index for type 1 packets
            } else if (packetType == 0x02) {
                index = index >> 1;  // Halve the index for type 2 packets
            }
            
            // Shift data to remove header
            for (size_t i = 0; i < size - 4; i++) {
                buffer[i] = buffer[i + 4];
            }
            size -= 4;
        }

        // Second transformation: Apply network protocol encoding
        if (size > 0) {
            // Transform index based on protocol encoding
            index = index ^ 0x55;  // Apply same XOR encoding to index
            
            for (size_t i = 0; i < size; i++) {
                // Simple XOR encoding with network protocol key
                buffer[i] = buffer[i] ^ 0x55;
            }
        }

        //SINK
        dest_buffer[index] = buf[0];  // Direct out-of-bounds write with attacker-controlled index
    }
}
} 