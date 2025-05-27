#ifndef SFML_SOCKETIMPL_HPP
#define SFML_SOCKETIMPL_HPP

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/Network/Export.hpp>
#include <SFML/Network/Socket.hpp>

#include <optional>

// Platform-specific headers
#if defined(SFML_SYSTEM_WINDOWS)
    #include <WinSock2.h>
    #include <WS2tcpip.h>
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
#endif

namespace sf
{
namespace priv
{
    ////////////////////////////////////////////////////////////
    /// \brief Helper class implementing socket functions
    ///
    ////////////////////////////////////////////////////////////
    class SFML_NETWORK_API SocketImpl
    {
    public:
        ////////////////////////////////////////////////////////////
        /// \brief Create a socket handle
        ///
        /// \return Socket handle
        ///
        ////////////////////////////////////////////////////////////
        static SocketHandle create();

        ////////////////////////////////////////////////////////////
        /// \brief Close a socket handle
        ///
        /// \param handle Socket handle to close
        ///
        ////////////////////////////////////////////////////////////
        static void close(SocketHandle handle);

        ////////////////////////////////////////////////////////////
        /// \brief Set a socket as blocking or non-blocking
        ///
        /// \param handle  Socket handle
        /// \param blocking True to set the socket as blocking, false for non-blocking
        ///
        /// \return True on success, false on failure
        ///
        ////////////////////////////////////////////////////////////
        static bool setBlocking(SocketHandle handle, bool blocking);

        ////////////////////////////////////////////////////////////
        /// \brief Get the last socket error status
        ///
        /// \return Status corresponding to the last socket error
        ///
        ////////////////////////////////////////////////////////////
        static Socket::Status getErrorStatus();

        ////////////////////////////////////////////////////////////
        /// \brief Get the invalid socket handle
        ///
        /// \return Invalid socket handle
        ///
        ////////////////////////////////////////////////////////////
        static SocketHandle invalidSocket();

        ////////////////////////////////////////////////////////////
        /// \brief Create an address structure
        ///
        /// \param address IP address
        /// \param port    Port number
        ///
        /// \return Address structure
        ///
        ////////////////////////////////////////////////////////////
        static sockaddr_in createAddress(std::uint32_t address, unsigned short port);
    };

} // namespace priv

} // namespace sf

#endif // SFML_SOCKETIMPL_HPP 