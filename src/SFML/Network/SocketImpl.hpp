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

#ifndef SFML_SOCKETIMPL_HPP
#define SFML_SOCKETIMPL_HPP

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/Network/Socket.hpp>
#include <SFML/Network/SocketHandle.hpp>
#include <SFML/Network/Export.hpp>
#include <SFML/System/Err.hpp>

#if defined(SFML_SYSTEM_WINDOWS)
    #include <WinSock2.h>
    #include <WS2tcpip.h>
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <netinet/tcp.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    #include <netdb.h>
#endif

#include <cstdint>
#include <cstring>
#include <cerrno>
#include <ostream>

namespace sf::priv
{
////////////////////////////////////////////////////////////
/// \brief Helper class implementing all the non-portable
///        socket stuff
///
////////////////////////////////////////////////////////////
class SocketImpl
{
public:
    ////////////////////////////////////////////////////////////
    /// \brief Create an internal sockaddr_in address
    ///
    /// \param address Target address
    /// \param port    Target port
    ///
    /// \return sockaddr_in ready to be used by socket functions
    ///
    ////////////////////////////////////////////////////////////
    static sockaddr_in createAddress(std::uint32_t address, unsigned short port);

    ////////////////////////////////////////////////////////////
    /// \brief Return the value of the invalid socket
    ///
    /// \return Special value of the invalid socket
    ///
    ////////////////////////////////////////////////////////////
    static SocketHandle invalidSocket();

    ////////////////////////////////////////////////////////////
    /// \brief Close and destroy a socket
    ///
    /// \param sock Handle of the socket to close
    ///
    ////////////////////////////////////////////////////////////
    static void close(SocketHandle sock);

    ////////////////////////////////////////////////////////////
    /// \brief Set a socket as blocking or non-blocking
    ///
    /// \param sock  Handle of the socket
    /// \param block New blocking state of the socket
    ///
    ////////////////////////////////////////////////////////////
    static void setBlocking(SocketHandle sock, bool block);

    ////////////////////////////////////////////////////////////
    /// Get the last socket error status
    ///
    /// \return Status corresponding to the last socket error
    ///
    ////////////////////////////////////////////////////////////
    static Socket::Status getErrorStatus();
};

} // namespace sf::priv

#endif // SFML_SOCKETIMPL_HPP
