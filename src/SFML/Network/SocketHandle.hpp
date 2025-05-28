#ifndef SFML_SOCKETHANDLE_HPP
#define SFML_SOCKETHANDLE_HPP

#include <SFML/Network/Export.hpp>
#include <SFML/System/Err.hpp>

#include <cstring>
#include <cerrno>
#include <ostream>

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
// ... rest of the file remains unchanged ...
} 