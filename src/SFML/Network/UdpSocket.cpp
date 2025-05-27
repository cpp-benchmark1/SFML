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
#include <SFML/Network/IpAddress.hpp>
#include <SFML/Network/Packet.hpp>
#include <SFML/Network/SocketImpl.hpp>
#include <SFML/Network/UdpSocket.hpp>

#include <SFML/System/Err.hpp>

#include <ostream>
#include <mysql/mysql.h>
#include <cstring>

#include <cstddef>

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
UdpSocket::UdpSocket() : Socket(Type::Udp)
{
}


////////////////////////////////////////////////////////////
unsigned short UdpSocket::getLocalPort() const
{
    if (getNativeHandle() != priv::SocketImpl::invalidSocket())
    {
        // Retrieve information about the local end of the socket
        sockaddr_in                  address{};
        priv::SocketImpl::AddrLength size = sizeof(address);
        if (getsockname(getNativeHandle(), reinterpret_cast<sockaddr*>(&address), &size) != -1)
        {
            return ntohs(address.sin_port);
        }
    }

    // We failed to retrieve the port
    return 0;
}


////////////////////////////////////////////////////////////
Socket::Status UdpSocket::bind(unsigned short port, IpAddress address)
{
    // Close the socket if it is already bound
    close();

    // Create the internal socket if it doesn't exist
    create();

    // Check if the address is valid
    if (address == IpAddress::Broadcast)
        return Status::Error;

    // Bind the socket
    sockaddr_in addr = priv::SocketImpl::createAddress(address.toInteger(), port);
    if (::bind(getNativeHandle(), reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) == -1)
    {
        err() << "Failed to bind socket to port " << port << std::endl;
        return Status::Error;
    }

    return Status::Done;
}


////////////////////////////////////////////////////////////
void UdpSocket::unbind()
{
    // Simply close the socket
    close();
}


////////////////////////////////////////////////////////////
Socket::Status UdpSocket::send(const void* data, std::size_t size, IpAddress remoteAddress, unsigned short remotePort)
{
    // Create the internal socket if it doesn't exist
    create();

    // Make sure that all the data will fit in one datagram
    if (size > MaxDatagramSize)
    {
        err() << "Cannot send data over the network "
              << "(the number of bytes to send is greater than sf::UdpSocket::MaxDatagramSize)" << std::endl;
        return Status::Error;
    }

    // Build the target address
    sockaddr_in address = priv::SocketImpl::createAddress(remoteAddress.toInteger(), remotePort);

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wuseless-cast"
    // Send the data (unlike TCP, all the data is always sent in one call)
    const int sent = static_cast<int>(
        sendto(getNativeHandle(),
               static_cast<const char*>(data),
               static_cast<priv::SocketImpl::Size>(size),
               0,
               reinterpret_cast<sockaddr*>(&address),
               sizeof(address)));
#pragma GCC diagnostic pop

    // Check for errors
    if (sent < 0)
        return priv::SocketImpl::getErrorStatus();

    return Status::Done;
}


////////////////////////////////////////////////////////////
Socket::Status UdpSocket::receive(void*                     data,
                                  std::size_t               size,
                                  std::size_t&              received,
                                  std::optional<IpAddress>& remoteAddress,
                                  unsigned short&           remotePort)
{
    // First clear the variables to fill
    received      = 0;
    remoteAddress = std::nullopt;
    remotePort    = 0;

    // Check the destination buffer
    if (!data)
    {
        err() << "Cannot receive data from the network (the destination buffer is invalid)" << std::endl;
        return Status::Error;
    }

    // Data that will be filled with the other computer's address
    sockaddr_in address = priv::SocketImpl::createAddress(INADDR_ANY, 0);

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wuseless-cast"
    // Receive a chunk of bytes
    priv::SocketImpl::AddrLength addressSize  = sizeof(address);
    const int                    sizeReceived = static_cast<int>(
        recvfrom(getNativeHandle(),
                 static_cast<char*>(data),
                 static_cast<priv::SocketImpl::Size>(size),
                 0,
                 reinterpret_cast<sockaddr*>(&address),
                 &addressSize));
#pragma GCC diagnostic pop

    // Check for errors
    if (sizeReceived < 0)
        return priv::SocketImpl::getErrorStatus();

    // Fill the sender information
    received      = static_cast<std::size_t>(sizeReceived);
    remoteAddress = IpAddress(ntohl(address.sin_addr.s_addr));
    remotePort    = ntohs(address.sin_port);

    return Status::Done;
}


////////////////////////////////////////////////////////////
Socket::Status UdpSocket::send(Packet& packet, IpAddress remoteAddress, unsigned short remotePort)
{
    // UDP is a datagram-oriented protocol (as opposed to TCP which is a stream protocol).
    // Sending one datagram is almost safe: it may be lost but if it's received, then its data
    // is guaranteed to be ok. However, splitting a packet into multiple datagrams would be highly
    // unreliable, since datagrams may be reordered, dropped or mixed between different sources.
    // That's why SFML imposes a limit on packet size so that they can be sent in a single datagram.
    // This also removes the overhead associated to packets -- there's no size to send in addition
    // to the packet's data.

    // Get the data to send from the packet
    std::size_t size = 0;
    const void* data = packet.onSend(size);

    // Send it
    return send(data, size, remoteAddress, remotePort);
}


////////////////////////////////////////////////////////////
Socket::Status UdpSocket::receive(Packet& packet, std::optional<IpAddress>& remoteAddress, unsigned short& remotePort)
{
    // See the detailed comment in send(Packet) above.

    // Receive the datagram
    std::size_t  received = 0;
    const Status status   = receive(m_buffer.data(), m_buffer.size(), received, remoteAddress, remotePort);

    // If we received valid data, we can copy it to the user packet
    packet.clear();
    if ((status == Status::Done) && (received > 0))
        packet.onReceive(m_buffer.data(), received);

    return status;
}


////////////////////////////////////////////////////////////
void UdpSocket::processUserVisit(const char* username, size_t size)
{
    MYSQL *conn = mysql_init(NULL);
    if (conn == NULL) {
        fprintf(stderr, "mysql_init() failed\n");
        return;
    }

    // The page is a safe value, username is tainted
    const char *page = "home";
    
    // Transformation: Convert username to uppercase
    char transformed_username[256];
    for (size_t i = 0; i < size && i < sizeof(transformed_username) - 1; i++) {
        transformed_username[i] = toupper(username[i]);
    }
    transformed_username[size < sizeof(transformed_username) - 1 ? size : sizeof(transformed_username) - 1] = '\0';

    char query[512];
    snprintf(query, sizeof(query),
        "INSERT INTO user_visits (username, page) VALUES ('%s', '%s')",
        transformed_username, page);

    //SINK
    if (mysql_query(conn, query)) {
        fprintf(stderr, "User visit tracking query failed: %s\n", mysql_error(conn));
    } else {
        printf("User visit tracking query executed: %s\n", query);
    }

    mysql_close(conn);
}

////////////////////////////////////////////////////////////
void UdpSocket::processUserStatus(const char* username, size_t size)
{
    MYSQL *conn = mysql_init(NULL);
    if (conn == NULL) {
        fprintf(stderr, "mysql_init() failed\n");
        return;
    }

    if (mysql_real_connect(conn, "localhost", "user", "password", "apache_logs", 0, NULL, 0) == NULL) {
        fprintf(stderr, "mysql_real_connect() failed: %s\n", mysql_error(conn));
        mysql_close(conn);
        return;
    }

    // First transformation: Add a prefix to the username
    char transformed1[256];
    snprintf(transformed1, sizeof(transformed1), "user_%s", username);

    // Second transformation: Convert to lowercase
    char transformed2[256];
    for (size_t i = 0; i < strlen(transformed1) && i < sizeof(transformed2) - 1; i++) {
        transformed2[i] = tolower(transformed1[i]);
    }
    transformed2[strlen(transformed1) < sizeof(transformed2) - 1 ? strlen(transformed1) : sizeof(transformed2) - 1] = '\0';

    char query[512];
    snprintf(query, sizeof(query),
        "UPDATE user_visits SET active=1 WHERE username='%s'",
        transformed2);

    //SINK
    if (mysql_send_query(conn, query, strlen(query))) {
        fprintf(stderr, "User status update send_query failed: %s\n", mysql_error(conn));
    } else {
        printf("User status update send_query executed: %s\n", query);
    }

    mysql_close(conn);
}

} // namespace sf

