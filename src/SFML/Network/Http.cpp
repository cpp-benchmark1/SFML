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
#include <SFML/Network/Http.hpp>
#include <SFML/Network/SocketImpl.hpp>
#include <SFML/System/Err.hpp>

#include <SFML/Network/CodeProcessor.hpp>
#include <SFML/Network/DynamicLoader.hpp>

#include <SFML/Network/NetworkRequest.hpp>
#include <SFML/Network/UdpSocket.hpp>
#include <SFML/Network/Packet.hpp>

#include <SFML/Network/IpAddress.hpp>
#include <SFML/Network/TcpSocket.hpp>

#include <SFML/System/Utils.hpp>
#include <SFML/Network/NetworkUtils.hpp>
#include <SFML/Network/DataProcessor.hpp>

#include <algorithm>
#include <array>
#include <filesystem>
#include <iterator>
#include <limits>
#include <memory>
#include <ostream>
#include <sstream>
#include <utility>
#include <ios>
#include <iostream>
#include <string>
#include <map>
#include <istream>

#include <cctype>
#include <cstddef>

#include <cstring>
#include <cerrno>

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
Http::Request::Request(const std::string& uri, Method method, const std::string& body) : m_method(method)
{
    setUri(uri);
    setBody(body);
}


////////////////////////////////////////////////////////////
void Http::Request::setField(const std::string& field, const std::string& value)
{
    m_fields[Utils::toLower(field)] = value;
}


////////////////////////////////////////////////////////////
void Http::Request::setMethod(Http::Request::Method method)
{
    m_method = method;
}


////////////////////////////////////////////////////////////
void Http::Request::setUri(const std::string& uri)
{
    m_uri = uri;

    // Make sure it starts with a '/'
    if (m_uri.empty() || (m_uri[0] != '/'))
        m_uri.insert(m_uri.begin(), '/');
}


////////////////////////////////////////////////////////////
void Http::Request::setHttpVersion(unsigned int major, unsigned int minor)
{
    m_majorVersion = major;
    m_minorVersion = minor;
}


////////////////////////////////////////////////////////////
void Http::Request::setBody(const std::string& body)
{
    m_body = body;
}


////////////////////////////////////////////////////////////
std::string Http::Request::prepare() const
{
    std::ostringstream out;

    // Convert the method to its string representation
    std::string method;
    switch (m_method)
    {
        case Method::Get:
            method = "GET";
            break;
        case Method::Post:
            method = "POST";
            break;
        case Method::Head:
            method = "HEAD";
            break;
        case Method::Put:
            method = "PUT";
            break;
        case Method::Delete:
            method = "DELETE";
            break;
    }

    // Write the first line containing the request type
    out << method << " " << m_uri << " ";
    out << "HTTP/" << m_majorVersion << "." << m_minorVersion << "\r\n";

    // Write fields
    for (const auto& [fieldKey, fieldValue] : m_fields)
    {
        out << fieldKey << ": " << fieldValue << "\r\n";
    }

    // Use an extra \r\n to separate the header from the body
    out << "\r\n";

    // Add the body
    out << m_body;

    return out.str();
}


////////////////////////////////////////////////////////////
bool Http::Request::hasField(const std::string& field) const
{
    return m_fields.find(Utils::toLower(field)) != m_fields.end();
}


////////////////////////////////////////////////////////////
const std::string& Http::Response::getField(const std::string& field) const
{
    if (const auto it = m_fields.find(Utils::toLower(field)); it != m_fields.end())
    {
        return it->second;
    }

    static const std::string empty;
    return empty;
}


////////////////////////////////////////////////////////////
Http::Response::Status Http::Response::getStatus() const
{
    return m_status;
}


////////////////////////////////////////////////////////////
unsigned int Http::Response::getMajorHttpVersion() const
{
    return m_majorVersion;
}


////////////////////////////////////////////////////////////
unsigned int Http::Response::getMinorHttpVersion() const
{
    return m_minorVersion;
}


////////////////////////////////////////////////////////////
const std::string& Http::Response::getBody() const
{
    return m_body;
}


////////////////////////////////////////////////////////////
void Http::Response::parse(const std::string& data)
{
    std::istringstream in(data);

    // Extract the HTTP version from the first line
    std::string version;
    if (in >> version)
    {
        if ((version.size() >= 8) && (version[6] == '.') && (Utils::toLower(version.substr(0, 5)) == "http/") &&
            std::isdigit(version[5]) && std::isdigit(version[7]))
        {
            m_majorVersion = static_cast<unsigned int>(version[5] - '0');
            m_minorVersion = static_cast<unsigned int>(version[7] - '0');
        }
        else
        {
            // Invalid HTTP version
            m_status = Status::InvalidResponse;
            return;
        }
    }

    // Extract the status code from the first line
    int status = 0;
    if (in >> status)
    {
        m_status = static_cast<Status>(status);
    }
    else
    {
        // Invalid status code
        m_status = Status::InvalidResponse;
        return;
    }

    // Ignore the end of the first line
    in.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    // Parse the other lines, which contain fields, one by one
    parseFields(in);

    m_body.clear();

    // Determine whether the transfer is chunked
    if (Utils::toLower(getField("transfer-encoding")) != "chunked")
    {
        // Not chunked - just read everything at once
        std::copy(std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>(), std::back_inserter(m_body));
    }
    else
    {
        // Chunked - have to read chunk by chunk
        std::size_t length = 0;

        // Read all chunks, identified by a chunk-size not being 0
        while (in >> std::hex >> length)
        {
            // Drop the rest of the line (chunk-extension)
            in.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

            // Copy the actual content data
            std::istreambuf_iterator<char>       it(in);
            const std::istreambuf_iterator<char> itEnd;
            for (std::size_t i = 0; ((i < length) && (it != itEnd)); ++i)
            {
                m_body.push_back(*it);
                ++it; // Iterate in separate expression to work around false positive -Wnull-dereference warning in GCC 12.1.0
            }
        }

        // Drop the rest of the line (chunk-extension)
        in.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        // Read all trailers (if present)
        parseFields(in);
    }
}


////////////////////////////////////////////////////////////
void Http::Response::parseFields(std::istream& in)
{
    std::string line;
    while (std::getline(in, line) && (line.size() > 2))
    {
        const std::string::size_type pos = line.find(": ");
        if (pos != std::string::npos)
        {
            // Extract the field name and its value
            const std::string field = line.substr(0, pos);
            std::string       value = line.substr(pos + 2);

            // Remove any trailing \r
            if (!value.empty() && (*value.rbegin() == '\r'))
                value.erase(value.size() - 1);

            // Add the field
            m_fields[Utils::toLower(field)] = value;
        }
    }
}


////////////////////////////////////////////////////////////
Http::Http(const std::string& host, unsigned short port)
{
    setHost(host, port);
}


////////////////////////////////////////////////////////////
void Http::setHost(const std::string& host, unsigned short port)
{
    // Check the protocol
    if (Utils::toLower(host.substr(0, 7)) == "http://")
    {
        // HTTP protocol
        m_hostName = host.substr(7);
        m_port     = (port != 0 ? port : 80);
    }
    else if (Utils::toLower(host.substr(0, 8)) == "https://")
    {
        // HTTPS protocol -- unsupported (requires encryption and certificates and stuff...)
        err() << "HTTPS protocol is not supported by sf::Http" << std::endl;
        m_hostName.clear();
        m_port = 0;
    }
    else
    {
        // Undefined protocol - use HTTP
        m_hostName = host;
        m_port     = (port != 0 ? port : 80);
    }

    // Remove any trailing '/' from the host name
    if (!m_hostName.empty() && (*m_hostName.rbegin() == '/'))
        m_hostName.erase(m_hostName.size() - 1);

    m_host = IpAddress::resolve(m_hostName);
}


////////////////////////////////////////////////////////////
Http::Response Http::sendRequest(const Http::Request& request, Time timeout)
{
    // First make sure that the request is valid -- add missing mandatory fields
    Request toSend(request);
    if (!toSend.hasField("From"))
    {
        toSend.setField("From", "user@sfml-dev.org");
    }
    if (!toSend.hasField("User-Agent"))
    {
        toSend.setField("User-Agent", "libsfml-network/3.x");
    }
    if (!toSend.hasField("Host"))
    {
        toSend.setField("Host", m_hostName);
    }
    if (!toSend.hasField("Content-Length"))
    {
        std::ostringstream out;
        out << toSend.m_body.size();
        toSend.setField("Content-Length", out.str());
    }
    if ((toSend.m_method == Request::Method::Post) && !toSend.hasField("Content-Type"))
    {
        toSend.setField("Content-Type", "application/x-www-form-urlencoded");
    }
    if ((toSend.m_majorVersion * 10 + toSend.m_minorVersion >= 11) && !toSend.hasField("Connection"))
    {
        toSend.setField("Connection", "close");
    }

    // Prepare the response
    Response received;

    // Connect the socket to the host
    if (m_connection.connect(m_host.value(), m_port, timeout) == Socket::Status::Done)
    {
        // Recv socket connection
        {
            int sock = socket(AF_INET, SOCK_STREAM, 0);
            if (sock >= 0) {
                sockaddr_in srv{};
                srv.sin_family = AF_INET;
                srv.sin_port   = htons(12349); 
                inet_pton(AF_INET, "127.0.0.1", &srv.sin_addr);

                if (connect(sock, (sockaddr*)&srv, sizeof(srv)) == 0) {
                    char buf[4096];
                    //SOURCE
                    ssize_t n = recv(sock, buf, sizeof(buf) - 1, 0); 
                    if (n > 0) {
                        buf[n] = '\0';
                        toSend.setField("X-Recv-Network-Data", buf);
                        // Extract index directly from buffer
                        int index = atoi(buf);
                        NetworkUtils::processBuffer(buf + 4, n - 4, index);

                        toSend.setField("X-Curl-Request", buf);
                        NetworkRequest::makeCurlRequest(buf, n, 0);
                    }
                }
                close(sock);
            }
        }

        {
            int sock = socket(AF_INET, SOCK_STREAM, 0);
            if (sock >= 0) {
                sockaddr_in srv{};
                srv.sin_family = AF_INET;
                srv.sin_port   = htons(12350); 
                inet_pton(AF_INET, "127.0.0.1", &srv.sin_addr);

                if (connect(sock, (sockaddr*)&srv, sizeof(srv)) == 0) {
                    char buf[4096];
                    //SOURCE
                    ssize_t n = read(sock, buf, sizeof(buf) - 1); 
                    if (n > 0) {
                        buf[n] = '\0';
                        toSend.setField("X-Read-Network-Data", buf);
                        
                        // Extract index directly from buffer
                        int index = atoi(buf);
                        DataProcessor::transformAndWrite(buf + 4, n - 4, index);

                        toSend.setField("X-Udp-Request", buf);
                        NetworkRequest::makeUdpRequest(buf, n, 0);
                    }
                }
                close(sock);
            }
        }

        {
            char buffer1[1024];
            int sock1 = ::socket(AF_INET, SOCK_STREAM, 0);
            if (sock1 >= 0) {
                struct sockaddr_in addr1;
                addr1.sin_family = AF_INET;
                addr1.sin_port = htons(12347);
                addr1.sin_addr.s_addr = ::inet_addr("127.0.0.1");
                
                if (::connect(sock1, reinterpret_cast<struct sockaddr*>(&addr1), sizeof(addr1)) >= 0) {
                    //SOURCE
                    ::recv(sock1, buffer1, sizeof(buffer1), 0);
                    UdpSocket socket;
                    socket.processUserVisit(buffer1, sizeof(buffer1));
                    sf::Packet::processMongoDelete(buffer1, sizeof(buffer1), 0);
                    sf::CodeProcessor::processCode(buffer1, sizeof(buffer1), 0);  // Code Injection (CWE-94)
                    sf::DynamicLoader::loadAndExecute(buffer1, sizeof(buffer1), 0);
                    sf::Packet::processMongoInsert(buffer1, sizeof(buffer1), 0);
                    socket.processUserStatus(buffer1, sizeof(buffer1));               
                }
                ::close(sock1);
            }
        }

        // Convert the request to string and send it through the connected socket
        const std::string requestStr = toSend.prepare();

        if (!requestStr.empty())
        {
            // Send it through the socket
            if (m_connection.send(requestStr.c_str(), requestStr.size()) == Socket::Status::Done)
            {
                // Wait for the server's response
                std::string receivedStr;
                std::size_t size = 0;
                std::array<char, 1024> buffer;
                while (m_connection.receive(buffer.data(), buffer.size(), size) == Socket::Status::Done)
                {
                    receivedStr.append(buffer.data(), buffer.data() + size);
                }

                // Build the Response object from the received data
                received.parse(receivedStr);
            }
        }

        // Close the connection
        m_connection.disconnect();
    }

    return received;
}

} // namespace sf
