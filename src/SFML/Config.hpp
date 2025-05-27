#ifndef SFML_CONFIG_HPP
#define SFML_CONFIG_HPP

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

#include <cstring>
#include <cerrno>
#include <ostream>

namespace sf
{
// ... rest of the file remains unchanged ...
} 