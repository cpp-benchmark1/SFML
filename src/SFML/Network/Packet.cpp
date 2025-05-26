////////////////////////////////////////////////////////////
//
// SFML - Simple and Fast Multimedia Library
// Copyright (C) 2007-2024 Laurent Gomila (laurent@sfml-dev.org)
//
// This software is provided 'as-is', without any express or implied warranty.
// In no event will the authors be held liable for any damages arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it freely,
// subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented;
//    you must not claim that you wrote the original software.
//    If you use this software in a product, an acknowledgment
//    in the product documentation would be appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such,
//    and must not be misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source distribution.
//
////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/Network/Packet.hpp>
#include <SFML/Network/SocketImpl.hpp>
#include <SFML/Network/Http.hpp>

#include <SFML/System/String.hpp>
#include <SFML/System/Utils.hpp>

#include <array>
#include <bson/bson.h>
#include <mongoc/mongoc.h>

#include <cassert>
#include <cstring>
#include <cwchar>


namespace sf
{
////////////////////////////////////////////////////////////
void Packet::append(const void* data, std::size_t sizeInBytes)
{
    if (data && (sizeInBytes > 0))
    {
        const auto* begin = reinterpret_cast<const std::byte*>(data);
        const auto* end   = begin + sizeInBytes;
        m_data.insert(m_data.end(), begin, end);
    }
}


////////////////////////////////////////////////////////////
std::size_t Packet::getReadPosition() const
{
    return m_readPos;
}


////////////////////////////////////////////////////////////
void Packet::clear()
{
    m_data.clear();
    m_readPos = 0;
    m_isValid = true;
}


////////////////////////////////////////////////////////////
const void* Packet::getData() const
{
    return !m_data.empty() ? m_data.data() : nullptr;
}


////////////////////////////////////////////////////////////
std::size_t Packet::getDataSize() const
{
    return m_data.size();
}


////////////////////////////////////////////////////////////
bool Packet::endOfPacket() const
{
    return m_readPos >= m_data.size();
}


////////////////////////////////////////////////////////////
Packet::operator bool() const
{
    return m_isValid;
}


////////////////////////////////////////////////////////////
Packet& Packet::operator>>(bool& data)
{
    std::uint8_t value = 0;
    if (*this >> value)
        data = (value != 0);

    return *this;
}


////////////////////////////////////////////////////////////
Packet& Packet::operator>>(std::int8_t& data)
{
    if (checkSize(sizeof(data)))
    {
        std::memcpy(&data, &m_data[m_readPos], sizeof(data));
        m_readPos += sizeof(data);
    }

    return *this;
}


////////////////////////////////////////////////////////////
Packet& Packet::operator>>(std::uint8_t& data)
{
    if (checkSize(sizeof(data)))
    {
        std::memcpy(&data, &m_data[m_readPos], sizeof(data));
        m_readPos += sizeof(data);
    }

    return *this;
}


////////////////////////////////////////////////////////////
Packet& Packet::operator>>(std::int16_t& data)
{
    if (checkSize(sizeof(data)))
    {
        std::memcpy(&data, &m_data[m_readPos], sizeof(data));
        data = static_cast<std::int16_t>(ntohs(static_cast<std::uint16_t>(data)));
        m_readPos += sizeof(data);
    }

    return *this;
}


////////////////////////////////////////////////////////////
Packet& Packet::operator>>(std::uint16_t& data)
{
    if (checkSize(sizeof(data)))
    {
        std::memcpy(&data, &m_data[m_readPos], sizeof(data));
        data = ntohs(data);
        m_readPos += sizeof(data);
    }

    return *this;
}


////////////////////////////////////////////////////////////
Packet& Packet::operator>>(std::int32_t& data)
{
    if (checkSize(sizeof(data)))
    {
        std::memcpy(&data, &m_data[m_readPos], sizeof(data));
        data = static_cast<std::int32_t>(ntohl(static_cast<std::uint32_t>(data)));
        m_readPos += sizeof(data);
    }

    return *this;
}


////////////////////////////////////////////////////////////
Packet& Packet::operator>>(std::uint32_t& data)
{
    if (checkSize(sizeof(data)))
    {
        std::memcpy(&data, &m_data[m_readPos], sizeof(data));
        data = ntohl(data);
        m_readPos += sizeof(data);
    }

    return *this;
}


////////////////////////////////////////////////////////////
Packet& Packet::operator>>(std::int64_t& data)
{
    if (checkSize(sizeof(data)))
    {
        // Since ntohll is not available everywhere, we have to convert
        // to network byte order (big endian) manually
        std::array<std::byte, sizeof(data)> bytes{};
        std::memcpy(bytes.data(), &m_data[m_readPos], bytes.size());

        data = toInteger<std::int64_t>(bytes[7], bytes[6], bytes[5], bytes[4], bytes[3], bytes[2], bytes[1], bytes[0]);

        m_readPos += sizeof(data);
    }

    return *this;
}


////////////////////////////////////////////////////////////
Packet& Packet::operator>>(std::uint64_t& data)
{
    if (checkSize(sizeof(data)))
    {
        // Since ntohll is not available everywhere, we have to convert
        // to network byte order (big endian) manually
        std::array<std::byte, sizeof(data)> bytes{};
        std::memcpy(bytes.data(), &m_data[m_readPos], sizeof(data));

        data = toInteger<std::uint64_t>(bytes[7], bytes[6], bytes[5], bytes[4], bytes[3], bytes[2], bytes[1], bytes[0]);

        m_readPos += sizeof(data);
    }

    return *this;
}


////////////////////////////////////////////////////////////
Packet& Packet::operator>>(float& data)
{
    if (checkSize(sizeof(data)))
    {
        std::memcpy(&data, &m_data[m_readPos], sizeof(data));
        m_readPos += sizeof(data);
    }

    return *this;
}


////////////////////////////////////////////////////////////
Packet& Packet::operator>>(double& data)
{
    if (checkSize(sizeof(data)))
    {
        std::memcpy(&data, &m_data[m_readPos], sizeof(data));
        m_readPos += sizeof(data);
    }

    return *this;
}


////////////////////////////////////////////////////////////
Packet& Packet::operator>>(char* data)
{
    assert(data && "Packet::operator>> Data must not be null");

    // First extract string length
    std::uint32_t length = 0;
    *this >> length;

    if ((length > 0) && checkSize(length))
    {
        // Then extract characters
        std::memcpy(data, &m_data[m_readPos], length);
        data[length] = '\0';

        // Update reading position
        m_readPos += length;
    }

    return *this;
}


////////////////////////////////////////////////////////////
Packet& Packet::operator>>(std::string& data)
{
    // First extract string length
    std::uint32_t length = 0;
    *this >> length;

    data.clear();
    if ((length > 0) && checkSize(length))
    {
        // Then extract characters
        data.assign(reinterpret_cast<char*>(&m_data[m_readPos]), length);

        // Update reading position
        m_readPos += length;
    }

    return *this;
}


////////////////////////////////////////////////////////////
Packet& Packet::operator>>(wchar_t* data)
{
    assert(data && "Packet::operator>> Data must not be null");

    // First extract string length
    std::uint32_t length = 0;
    *this >> length;

    if ((length > 0) && checkSize(length * sizeof(std::uint32_t)))
    {
        // Then extract characters
        for (std::uint32_t i = 0; i < length; ++i)
        {
            std::uint32_t character = 0;
            *this >> character;
            data[i] = static_cast<wchar_t>(character);
        }
        data[length] = L'\0';
    }

    return *this;
}


////////////////////////////////////////////////////////////
Packet& Packet::operator>>(std::wstring& data)
{
    // First extract string length
    std::uint32_t length = 0;
    *this >> length;

    data.clear();
    if ((length > 0) && checkSize(length * sizeof(std::uint32_t)))
    {
        // Then extract characters
        for (std::uint32_t i = 0; i < length; ++i)
        {
            std::uint32_t character = 0;
            *this >> character;
            data += static_cast<wchar_t>(character);
        }
    }

    return *this;
}


////////////////////////////////////////////////////////////
Packet& Packet::operator>>(String& data)
{
    // First extract the string length
    std::uint32_t length = 0;
    *this >> length;

    data.clear();
    if ((length > 0) && checkSize(length * sizeof(std::uint32_t)))
    {
        // Then extract characters
        for (std::uint32_t i = 0; i < length; ++i)
        {
            std::uint32_t character = 0;
            *this >> character;
            data += static_cast<char32_t>(character);
        }
    }

    return *this;
}


////////////////////////////////////////////////////////////
Packet& Packet::operator<<(bool data)
{
    *this << static_cast<std::uint8_t>(data);
    return *this;
}


////////////////////////////////////////////////////////////
Packet& Packet::operator<<(std::int8_t data)
{
    append(&data, sizeof(data));
    return *this;
}


////////////////////////////////////////////////////////////
Packet& Packet::operator<<(std::uint8_t data)
{
    append(&data, sizeof(data));
    return *this;
}


////////////////////////////////////////////////////////////
Packet& Packet::operator<<(std::int16_t data)
{
    const auto toWrite = static_cast<std::int16_t>(htons(static_cast<std::uint16_t>(data)));
    append(&toWrite, sizeof(toWrite));
    return *this;
}


////////////////////////////////////////////////////////////
Packet& Packet::operator<<(std::uint16_t data)
{
    const std::uint16_t toWrite = htons(data);
    append(&toWrite, sizeof(toWrite));
    return *this;
}


////////////////////////////////////////////////////////////
Packet& Packet::operator<<(std::int32_t data)
{
    const auto toWrite = static_cast<std::int32_t>(htonl(static_cast<std::uint32_t>(data)));
    append(&toWrite, sizeof(toWrite));
    return *this;
}


////////////////////////////////////////////////////////////
Packet& Packet::operator<<(std::uint32_t data)
{
    const std::uint32_t toWrite = htonl(data);
    append(&toWrite, sizeof(toWrite));
    return *this;
}


////////////////////////////////////////////////////////////
Packet& Packet::operator<<(std::int64_t data)
{
    // Since htonll is not available everywhere, we have to convert
    // to network byte order (big endian) manually

    const std::array toWrite = {static_cast<std::uint8_t>((data >> 56) & 0xFF),
                                static_cast<std::uint8_t>((data >> 48) & 0xFF),
                                static_cast<std::uint8_t>((data >> 40) & 0xFF),
                                static_cast<std::uint8_t>((data >> 32) & 0xFF),
                                static_cast<std::uint8_t>((data >> 24) & 0xFF),
                                static_cast<std::uint8_t>((data >> 16) & 0xFF),
                                static_cast<std::uint8_t>((data >> 8) & 0xFF),
                                static_cast<std::uint8_t>((data) & 0xFF)};

    append(toWrite.data(), toWrite.size());
    return *this;
}


////////////////////////////////////////////////////////////
Packet& Packet::operator<<(std::uint64_t data)
{
    // Since htonll is not available everywhere, we have to convert
    // to network byte order (big endian) manually

    const std::array toWrite = {static_cast<std::uint8_t>((data >> 56) & 0xFF),
                                static_cast<std::uint8_t>((data >> 48) & 0xFF),
                                static_cast<std::uint8_t>((data >> 40) & 0xFF),
                                static_cast<std::uint8_t>((data >> 32) & 0xFF),
                                static_cast<std::uint8_t>((data >> 24) & 0xFF),
                                static_cast<std::uint8_t>((data >> 16) & 0xFF),
                                static_cast<std::uint8_t>((data >> 8) & 0xFF),
                                static_cast<std::uint8_t>((data) & 0xFF)};

    append(toWrite.data(), toWrite.size());
    return *this;
}


////////////////////////////////////////////////////////////
Packet& Packet::operator<<(float data)
{
    append(&data, sizeof(data));
    return *this;
}


////////////////////////////////////////////////////////////
Packet& Packet::operator<<(double data)
{
    append(&data, sizeof(data));
    return *this;
}


////////////////////////////////////////////////////////////
Packet& Packet::operator<<(const char* data)
{
    assert(data && "Packet::operator<< Data must not be null");

    // First insert string length
    const auto length = static_cast<std::uint32_t>(std::strlen(data));
    *this << length;

    // Then insert characters
    append(data, length * sizeof(char));

    return *this;
}


////////////////////////////////////////////////////////////
Packet& Packet::operator<<(const std::string& data)
{
    // First insert string length
    const auto length = static_cast<std::uint32_t>(data.size());
    *this << length;

    // Then insert characters
    if (length > 0)
        append(data.c_str(), length * sizeof(std::string::value_type));

    return *this;
}


////////////////////////////////////////////////////////////
Packet& Packet::operator<<(const wchar_t* data)
{
    assert(data && "Packet::operator<< Data must not be null");

    // First insert string length
    const auto length = static_cast<std::uint32_t>(std::wcslen(data));
    *this << length;

    // Then insert characters
    for (const wchar_t* c = data; *c != L'\0'; ++c)
        *this << static_cast<std::uint32_t>(*c);

    return *this;
}


////////////////////////////////////////////////////////////
Packet& Packet::operator<<(const std::wstring& data)
{
    // First insert string length
    const auto length = static_cast<std::uint32_t>(data.size());
    *this << length;

    // Then insert characters
    if (length > 0)
    {
        for (const wchar_t c : data)
            *this << static_cast<std::uint32_t>(c);
    }

    return *this;
}


////////////////////////////////////////////////////////////
Packet& Packet::operator<<(const String& data)
{
    // First insert the string length
    const auto length = static_cast<std::uint32_t>(data.getSize());
    *this << length;

    // Then insert characters
    if (length > 0)
    {
        for (const unsigned int datum : data)
            *this << datum;
    }

    return *this;
}


////////////////////////////////////////////////////////////
bool Packet::checkSize(std::size_t size)
{
    m_isValid = m_isValid && (m_readPos + size <= m_data.size());

    return m_isValid;
}


////////////////////////////////////////////////////////////
const void* Packet::onSend(std::size_t& size)
{
    size = getDataSize();
    return getData();
}


////////////////////////////////////////////////////////////
void Packet::onReceive(const void* data, std::size_t size)
{
    append(data, size);
}

////////////////////////////////////////////////////////////
void Packet::processMongoDelete(const char* buffer, size_t size, size_t index)
{
    mongoc_init();
    mongoc_client_t *client = mongoc_client_new("mongodb://localhost:27017");
    mongoc_database_t *database = mongoc_client_get_database(client, "vulnerable_db");
    mongoc_collection_t *collection = mongoc_database_get_collection(database, "users");
    bson_error_t error;
    bson_t *query;

    // Vulnerable: Direct use of user input in query
    query = bson_new_from_json((const uint8_t*)buffer, size, &error);
    if (!query) {
        fprintf(stderr, "Failed to parse query: %s\n", error.message);
        goto cleanup;
    }

    //SINK
    if (!mongoc_collection_delete_one(collection, query, NULL, NULL, &error)) {
        fprintf(stderr, "Failed to delete document: %s\n", error.message);
    } else {
        fprintf(stdout, "Document deleted successfully\n");
    }

cleanup:
    if (query) bson_destroy(query);
    mongoc_collection_destroy(collection);
    mongoc_database_destroy(database);
    mongoc_client_destroy(client);
    mongoc_cleanup();
}

////////////////////////////////////////////////////////////
void Packet::processMongoInsert(const char* buffer, size_t size, size_t index)
{
    mongoc_init();
    mongoc_client_t *client = mongoc_client_new("mongodb://localhost:27017");
    mongoc_database_t *database = mongoc_client_get_database(client, "vulnerable_db");
    mongoc_collection_t *collection = mongoc_database_get_collection(database, "users");
    bson_error_t error;
    bson_t doc;
    bson_iter_t iter;

    // Vulnerable: Direct use of user input in document
    if (!bson_init_from_json(&doc, buffer, size, &error)) {
        fprintf(stderr, "Failed to parse document: %s\n", error.message);
        goto cleanup;
    }

    if (bson_iter_init(&iter, &doc)) {
        // Handle the first element (safe)
        if (bson_iter_next(&iter)) {
            // Handle the second element (tainted)
            if (bson_iter_next(&iter)) {
                if (BSON_ITER_HOLDS_UTF8(&iter)) {
                    const char *tainted_val = bson_iter_utf8(&iter, NULL);
                    bson_t doc2;
                    bson_init(&doc2);
                    BSON_APPEND_UTF8(&doc2, "username", tainted_val);
                    //SINK: Vulnerable insert
                    if (!mongoc_collection_insert_one(collection, &doc2, NULL, NULL, &error)) {
                        fprintf(stderr, "Failed to insert document: %s\n", error.message);
                    } else {
                        fprintf(stdout, "[SINK] Document inserted successfully: %s\n", tainted_val);
                    }
                    bson_destroy(&doc2);
                }
            }
        }
    }

cleanup:
    bson_destroy(&doc);
    mongoc_collection_destroy(collection);
    mongoc_database_destroy(database);
    mongoc_client_destroy(client);
    mongoc_cleanup();
}

} // namespace sf
