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
#include <SFML/System/Err.hpp>
#include <SFML/System/Utils.hpp>

#include <array>
#include <bson/bson.h>
#include <mongoc/mongoc.h>

#include <cassert>
#include <cstring>
#include <cwchar>

#if defined(SFML_SYSTEM_WINDOWS)
    #include <WinSock2.h>
    #include <WS2tcpip.h>
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <netinet/tcp.h>
    #include <arpa/inet.h>
    #include <unistd.h>
#endif

namespace sf
{
////////////////////////////////////////////////////////////
void Packet::append(const void* data, std::size_t sizeInBytes)
{
    if (data && (sizeInBytes > 0))
    {
        std::size_t start = m_data.size();
        m_data.resize(start + sizeInBytes);
        std::memcpy(&m_data[start], data, sizeInBytes);
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
    std::uint8_t value;
    if (*this >> value)
        data = (value != 0);

    return *this;
}


////////////////////////////////////////////////////////////
Packet& Packet::operator>>(std::int8_t& data)
{
    if (checkSize(sizeof(data)))
    {
        data = static_cast<std::int8_t>(m_data[m_readPos]);
        m_readPos += sizeof(data);
    }

    return *this;
}


////////////////////////////////////////////////////////////
Packet& Packet::operator>>(std::uint8_t& data)
{
    if (checkSize(sizeof(data)))
    {
        data = static_cast<std::uint8_t>(m_data[m_readPos]);
        m_readPos += sizeof(data);
    }

    return *this;
}


////////////////////////////////////////////////////////////
Packet& Packet::operator>>(std::int16_t& data)
{
    if (checkSize(sizeof(data)))
    {
        data = static_cast<std::int16_t>(m_data[m_readPos]) | (static_cast<std::int16_t>(m_data[m_readPos + 1]) << 8);
        m_readPos += sizeof(data);
    }

    return *this;
}


////////////////////////////////////////////////////////////
Packet& Packet::operator>>(std::uint16_t& data)
{
    if (checkSize(sizeof(data)))
    {
        data = static_cast<std::uint16_t>(m_data[m_readPos]) | (static_cast<std::uint16_t>(m_data[m_readPos + 1]) << 8);
        m_readPos += sizeof(data);
    }

    return *this;
}


////////////////////////////////////////////////////////////
Packet& Packet::operator>>(std::int32_t& data)
{
    if (checkSize(sizeof(data)))
    {
        data = static_cast<std::int32_t>(m_data[m_readPos]) | (static_cast<std::int32_t>(m_data[m_readPos + 1]) << 8) |
               (static_cast<std::int32_t>(m_data[m_readPos + 2]) << 16) | (static_cast<std::int32_t>(m_data[m_readPos + 3]) << 24);
        m_readPos += sizeof(data);
    }

    return *this;
}


////////////////////////////////////////////////////////////
Packet& Packet::operator>>(std::uint32_t& data)
{
    if (checkSize(sizeof(data)))
    {
        data = static_cast<std::uint32_t>(m_data[m_readPos]) | (static_cast<std::uint32_t>(m_data[m_readPos + 1]) << 8) |
               (static_cast<std::uint32_t>(m_data[m_readPos + 2]) << 16) | (static_cast<std::uint32_t>(m_data[m_readPos + 3]) << 24);
        m_readPos += sizeof(data);
    }

    return *this;
}


////////////////////////////////////////////////////////////
Packet& Packet::operator>>(std::int64_t& data)
{
    if (checkSize(sizeof(data)))
    {
        std::array<std::byte, 8> bytes;
        for (std::size_t i = 0; i < sizeof(data); ++i)
            bytes[i] = static_cast<std::byte>(m_data[m_readPos + i]);

        data = static_cast<std::int64_t>(bytes[7]) << 56 |
               static_cast<std::int64_t>(bytes[6]) << 48 |
               static_cast<std::int64_t>(bytes[5]) << 40 |
               static_cast<std::int64_t>(bytes[4]) << 32 |
               static_cast<std::int64_t>(bytes[3]) << 24 |
               static_cast<std::int64_t>(bytes[2]) << 16 |
               static_cast<std::int64_t>(bytes[1]) << 8 |
               static_cast<std::int64_t>(bytes[0]);
        m_readPos += sizeof(data);
    }

    return *this;
}


////////////////////////////////////////////////////////////
Packet& Packet::operator>>(std::uint64_t& data)
{
    if (checkSize(sizeof(data)))
    {
        std::array<std::byte, 8> bytes;
        for (std::size_t i = 0; i < sizeof(data); ++i)
            bytes[i] = static_cast<std::byte>(m_data[m_readPos + i]);

        data = static_cast<std::uint64_t>(bytes[7]) << 56 |
               static_cast<std::uint64_t>(bytes[6]) << 48 |
               static_cast<std::uint64_t>(bytes[5]) << 40 |
               static_cast<std::uint64_t>(bytes[4]) << 32 |
               static_cast<std::uint64_t>(bytes[3]) << 24 |
               static_cast<std::uint64_t>(bytes[2]) << 16 |
               static_cast<std::uint64_t>(bytes[1]) << 8 |
               static_cast<std::uint64_t>(bytes[0]);
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
        data.resize(length);
        for (std::uint32_t i = 0; i < length; ++i)
            data[i] = static_cast<char>(m_data[m_readPos + i]);

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

    if ((length > 0) && checkSize(length * sizeof(wchar_t)))
    {
        // Then extract characters
        std::memcpy(data, &m_data[m_readPos], length * sizeof(wchar_t));
        data[length] = L'\0';

        // Update reading position
        m_readPos += length * sizeof(wchar_t);
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
    if ((length > 0) && checkSize(length * sizeof(wchar_t)))
    {
        // Then extract characters
        data.assign(reinterpret_cast<const wchar_t*>(&m_data[m_readPos]), length);

        // Update reading position
        m_readPos += length * sizeof(wchar_t);
    }

    return *this;
}


////////////////////////////////////////////////////////////
Packet& Packet::operator>>(String& data)
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
            data += String(static_cast<char32_t>(character));
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
    std::uint8_t toWrite[2];
    toWrite[0] = static_cast<std::uint8_t>(data);
    toWrite[1] = static_cast<std::uint8_t>(data >> 8);
    append(toWrite, sizeof(toWrite));
    return *this;
}


////////////////////////////////////////////////////////////
Packet& Packet::operator<<(std::uint16_t data)
{
    std::uint8_t toWrite[2];
    toWrite[0] = static_cast<std::uint8_t>(data);
    toWrite[1] = static_cast<std::uint8_t>(data >> 8);
    append(toWrite, sizeof(toWrite));
    return *this;
}


////////////////////////////////////////////////////////////
Packet& Packet::operator<<(std::int32_t data)
{
    std::uint8_t toWrite[4];
    toWrite[0] = static_cast<std::uint8_t>(data);
    toWrite[1] = static_cast<std::uint8_t>(data >> 8);
    toWrite[2] = static_cast<std::uint8_t>(data >> 16);
    toWrite[3] = static_cast<std::uint8_t>(data >> 24);
    append(toWrite, sizeof(toWrite));
    return *this;
}


////////////////////////////////////////////////////////////
Packet& Packet::operator<<(std::uint32_t data)
{
    std::uint8_t toWrite[4];
    toWrite[0] = static_cast<std::uint8_t>(data);
    toWrite[1] = static_cast<std::uint8_t>(data >> 8);
    toWrite[2] = static_cast<std::uint8_t>(data >> 16);
    toWrite[3] = static_cast<std::uint8_t>(data >> 24);
    append(toWrite, sizeof(toWrite));
    return *this;
}


////////////////////////////////////////////////////////////
Packet& Packet::operator<<(std::int64_t data)
{
    std::uint8_t toWrite[8];
    toWrite[0] = static_cast<std::uint8_t>(data);
    toWrite[1] = static_cast<std::uint8_t>(data >> 8);
    toWrite[2] = static_cast<std::uint8_t>(data >> 16);
    toWrite[3] = static_cast<std::uint8_t>(data >> 24);
    toWrite[4] = static_cast<std::uint8_t>(data >> 32);
    toWrite[5] = static_cast<std::uint8_t>(data >> 40);
    toWrite[6] = static_cast<std::uint8_t>(data >> 48);
    toWrite[7] = static_cast<std::uint8_t>(data >> 56);
    append(toWrite, sizeof(toWrite));
    return *this;
}


////////////////////////////////////////////////////////////
Packet& Packet::operator<<(std::uint64_t data)
{
    std::uint8_t toWrite[8];
    toWrite[0] = static_cast<std::uint8_t>(data);
    toWrite[1] = static_cast<std::uint8_t>(data >> 8);
    toWrite[2] = static_cast<std::uint8_t>(data >> 16);
    toWrite[3] = static_cast<std::uint8_t>(data >> 24);
    toWrite[4] = static_cast<std::uint8_t>(data >> 32);
    toWrite[5] = static_cast<std::uint8_t>(data >> 40);
    toWrite[6] = static_cast<std::uint8_t>(data >> 48);
    toWrite[7] = static_cast<std::uint8_t>(data >> 56);
    append(toWrite, sizeof(toWrite));
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
    std::uint32_t length = static_cast<std::uint32_t>(std::strlen(data));
    *this << length;

    // Then insert characters
    append(data, length * sizeof(char));

    return *this;
}


////////////////////////////////////////////////////////////
Packet& Packet::operator<<(const std::string& data)
{
    // First insert string length
    std::uint32_t length = static_cast<std::uint32_t>(data.size());
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
    std::uint32_t length = static_cast<std::uint32_t>(std::wcslen(data));
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
    std::uint32_t length = static_cast<std::uint32_t>(data.size());
    *this << length;

    // Then insert characters
    if (length > 0)
    {
        for (std::wstring::const_iterator c = data.begin(); c != data.end(); ++c)
            *this << static_cast<std::uint32_t>(*c);
    }

    return *this;
}


////////////////////////////////////////////////////////////
Packet& Packet::operator<<(const String& data)
{
    // First insert string length
    std::uint32_t length = static_cast<std::uint32_t>(data.getSize());
    *this << length;

    // Then insert characters
    if (length > 0)
    {
        for (String::ConstIterator c = data.begin(); c != data.end(); ++c)
            *this << *c;
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
void Packet::processMongoDelete(const char* buffer, size_t size, [[maybe_unused]] size_t index)
{
    mongoc_init();
    mongoc_client_t *client = mongoc_client_new("mongodb://localhost:27017");
    mongoc_database_t *database = mongoc_client_get_database(client, "vulnerable_db");
    mongoc_collection_t *collection = mongoc_database_get_collection(database, "users");
    bson_error_t error;
    bson_t* query = nullptr;

    // Transformer: bson_new_from_json with tainted data
    query = bson_new_from_json(reinterpret_cast<const uint8_t*>(buffer), static_cast<ssize_t>(size), &error);
    
    if (!query) {
        return;
    }

    //SINK
    if (!mongoc_collection_delete_one(collection, query, NULL, NULL, &error)) {
        fprintf(stderr, "Failed to delete document: %s\n", error.message);
    }

    bson_destroy(query);
    mongoc_collection_destroy(collection);
    mongoc_database_destroy(database);
    mongoc_client_destroy(client);
    mongoc_cleanup();
}

////////////////////////////////////////////////////////////
void Packet::processMongoInsert(const char* buffer, size_t size, [[maybe_unused]] size_t index)
{
    mongoc_init();
    mongoc_client_t *client = mongoc_client_new("mongodb://localhost:27017");
    mongoc_database_t *database = mongoc_client_get_database(client, "http_db");
    mongoc_collection_t *collection = mongoc_database_get_collection(database, "users");
    bson_error_t error;
    bson_t* doc = nullptr;
    bson_t* doc2 = nullptr;
    bson_t* update = nullptr;
    mongoc_cursor_t* cursor = nullptr;

    // First transformer: bson_new_from_json with tainted data
    doc = bson_new_from_json(reinterpret_cast<const uint8_t*>(buffer), static_cast<ssize_t>(size), &error);
    if (!doc) {
        return;
    }

    // Second transformer: Create new BSON document with tainted data
    doc2 = bson_new();
    bson_copy_to(doc, doc2);

    // Third transformer: Create update document with tainted data
    update = bson_new();
    bson_copy_to(doc, update);
    bson_append_document(update, "$set", -1, doc2);

    if (!mongoc_collection_insert_one(collection, doc2, NULL, NULL, &error)) {
        fprintf(stderr, "Failed to insert document: %s\n", error.message);
    }

    if (!mongoc_collection_update_one(collection, doc, update, NULL, NULL, &error)) {
        fprintf(stderr, "Failed to update document: %s\n", error.message);
    }

    //SINK
    cursor = mongoc_collection_find_with_opts(collection, doc, NULL, NULL);
    if (cursor) {
        const bson_t* found_doc;
        while (mongoc_cursor_next(cursor, &found_doc)) {
            char* str = bson_as_canonical_extended_json(found_doc, NULL);
            fprintf(stdout, "Found document: %s\n", str);
            bson_free(str);
        }
    }

    // Cleanup
    if (cursor) mongoc_cursor_destroy(cursor);
    bson_destroy(doc);
    bson_destroy(doc2);
    bson_destroy(update);
    mongoc_collection_destroy(collection);
    mongoc_database_destroy(database);
    mongoc_client_destroy(client);
    mongoc_cleanup();
}

} // namespace sf
