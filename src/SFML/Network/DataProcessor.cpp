#include <SFML/Network/DataProcessor.hpp>
#include <cstring>

namespace sf
{
namespace DataProcessor
{
    void transformAndWrite(char* buffer, size_t size, size_t index)
    {
        // First transformation: Process network packet headers
        // Extract packet type and shift data
        if (size > 4) {
            unsigned char packetType = buffer[0];
            unsigned short dataLength = (buffer[1] << 8) | buffer[2];
            
            // Shift data to remove header
            for (size_t i = 0; i < size - 4; i++) {
                buffer[i] = buffer[i + 4];
            }
            size -= 4;
            
            // Apply packet-specific processing
            if (packetType == 0x01) {
                // Type 1: Apply bit rotation
                for (size_t i = 0; i < size; i++) {
                    unsigned char byte = buffer[i];
                    buffer[i] = (byte << 4) | (byte >> 4);
                }
            }
        }

        // Second transformation: Process payload data
        // Decode and transform the message content
        if (size > 0) {
            // Apply message decoding
            for (size_t i = 0; i < size; i++) {
                // Decode using a simple substitution cipher
                buffer[i] = ((buffer[i] - 32 + 95) % 95) + 32;
            }
        }

        // Attacker-controlled index write
        //SINK
        buffer[index] = 'A';  // Single byte write with attacker-controlled index
    }
}
} 