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
#include <SFML/Network/IpAddress.hpp>
#include <SFML/Network/SocketImpl.hpp>

#include <SFML/System/Err.hpp>

#include <istream>
#include <ostream>

#include <cstring>
#include <sstream>

#if defined(SFML_SYSTEM_WINDOWS)
    #include <WinSock2.h>
    #include <WS2tcpip.h>
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <netinet/tcp.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    #include <netdb.h>  // For getaddrinfo, freeaddrinfo, addrinfo
#endif

namespace sf
{
////////////////////////////////////////////////////////////
const IpAddress IpAddress::None;
const IpAddress IpAddress::Any(0, 0, 0, 0);
const IpAddress IpAddress::LocalHost(127, 0, 0, 1);
const IpAddress IpAddress::Broadcast(255, 255, 255, 255);


////////////////////////////////////////////////////////////
IpAddress::IpAddress() : m_address(0)
{
}


////////////////////////////////////////////////////////////
IpAddress::IpAddress(std::uint8_t byte0, std::uint8_t byte1, std::uint8_t byte2, std::uint8_t byte3) :
    m_address(htonl((byte0 << 24) | (byte1 << 16) | (byte2 << 8) | byte3))
{
}


////////////////////////////////////////////////////////////
IpAddress::IpAddress(std::uint32_t address) : m_address(htonl(address))
{
}


////////////////////////////////////////////////////////////
std::optional<IpAddress> IpAddress::resolve(std::string_view address)
{
    if (address.empty())
        return std::nullopt;

    // Try to convert the address as a byte representation ("xxx.xxx.xxx.xxx")
    if (const std::uint32_t ip = inet_addr(address.data()); ip != INADDR_NONE)
        return IpAddress(ntohl(ip));

    // Not a valid address, try to convert it as a host name
    addrinfo hints{}; // Zero-initialize
    hints.ai_family = AF_INET;

    addrinfo* result = nullptr;
    if (getaddrinfo(address.data(), nullptr, &hints, &result) == 0 && result != nullptr)
    {
        sockaddr_in sin{};
        std::memcpy(&sin, result->ai_addr, sizeof(*result->ai_addr));
        const std::uint32_t ip = sin.sin_addr.s_addr;

        freeaddrinfo(result);

        return IpAddress(ntohl(ip));
    }

    return std::nullopt;
}


////////////////////////////////////////////////////////////
std::string IpAddress::toString() const
{
    in_addr address{};
    address.s_addr = htonl(m_address);

    return inet_ntoa(address);
}


////////////////////////////////////////////////////////////
std::uint32_t IpAddress::toInteger() const
{
    return ntohl(m_address);
}


////////////////////////////////////////////////////////////
std::optional<IpAddress> IpAddress::getLocalAddress()
{
    // The method here is to connect a UDP socket to anyone (here, localhost),
    // and get the local socket address with the system function getsockname.
    // This is the only way to get the local address, as there is no reliable
    // cross-platform way to get it directly.

    // Create the socket
    const SocketHandle sock = socket(PF_INET, SOCK_DGRAM, 0);
    if (sock == priv::SocketImpl::invalidSocket())
        return std::nullopt;

    // Connect the socket to localhost on any port
    sockaddr_in address = priv::SocketImpl::createAddress(ntohl(INADDR_LOOPBACK), 9);
    if (connect(sock, reinterpret_cast<struct sockaddr*>(&address), sizeof(address)) == -1)
    {
        priv::SocketImpl::close(sock);
        return std::nullopt;
    }

    // Get the local address of the socket connection
    socklen_t size = sizeof(address);
    if (getsockname(sock, reinterpret_cast<struct sockaddr*>(&address), &size) == -1)
    {
        priv::SocketImpl::close(sock);
        return std::nullopt;
    }

    // Close the socket
    priv::SocketImpl::close(sock);

    // Finally build the IP address
    return IpAddress(ntohl(address.sin_addr.s_addr));
}


////////////////////////////////////////////////////////////
std::optional<IpAddress> IpAddress::getPublicAddress(Time timeout)
{
    // The trick here is more complicated, because the only way
    // to get our public IP address is to get it from a distant computer.
    // Here we get the web page from http://www.sfml-dev.org/ip-provider.php
    // and parse the result to extract our IP address
    // (not very hard: the web page contains only our IP address).

    Http                 server("www.sfml-dev.org");
    const Http::Request  request("/ip-provider.php", Http::Request::Method::Get);
    const Http::Response page = server.sendRequest(request, timeout);

    const Http::Response::Status status = page.getStatus();

    if (status == Http::Response::Status::Ok)
        return IpAddress::resolve(page.getBody());

    err() << "Failed to retrieve public address from external IP resolution server (HTTP response status "
          << static_cast<int>(status) << ")" << std::endl;

    return std::nullopt;
}


////////////////////////////////////////////////////////////
bool operator==(const IpAddress& left, const IpAddress& right)
{
    return left.toInteger() == right.toInteger();
}


////////////////////////////////////////////////////////////
bool operator!=(const IpAddress& left, const IpAddress& right)
{
    return !(left == right);
}


////////////////////////////////////////////////////////////
bool operator<(const IpAddress& left, const IpAddress& right)
{
    return left.toInteger() < right.toInteger();
}


////////////////////////////////////////////////////////////
bool operator>(const IpAddress& left, const IpAddress& right)
{
    return right < left;
}


////////////////////////////////////////////////////////////
bool operator<=(const IpAddress& left, const IpAddress& right)
{
    return !(right < left);
}


////////////////////////////////////////////////////////////
bool operator>=(const IpAddress& left, const IpAddress& right)
{
    return !(left < right);
}


////////////////////////////////////////////////////////////
std::istream& operator>>(std::istream& stream, IpAddress& address)
{
    std::string str;
    stream >> str;
    address = IpAddress::resolve(str).value_or(IpAddress::None);

    return stream;
}


////////////////////////////////////////////////////////////
std::ostream& operator<<(std::ostream& stream, const IpAddress& address)
{
    return stream << address.toString();
}


////////////////////////////////////////////////////////////
std::istream& operator>>(std::istream& stream, std::optional<IpAddress>& address)
{
    std::string str;
    stream >> str;
    address = IpAddress::resolve(str);
    return stream;
}

} // namespace sf
