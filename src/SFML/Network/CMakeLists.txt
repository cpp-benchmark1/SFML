set(INCROOT ${PROJECT_SOURCE_DIR}/include/SFML/Network)
set(SRCROOT ${PROJECT_SOURCE_DIR}/src/SFML/Network)

# Find MongoDB packages
find_package(bson-1.0 REQUIRED)
find_package(mongoc-1.0 REQUIRED)

# all source files
set(SRC
    ${INCROOT}/Export.hpp
    ${SRCROOT}/Ftp.cpp
    ${INCROOT}/Ftp.hpp
    ${SRCROOT}/Http.cpp
    ${INCROOT}/Http.hpp
    ${SRCROOT}/IpAddress.cpp
    ${INCROOT}/IpAddress.hpp
    ${SRCROOT}/Packet.cpp
    ${INCROOT}/Packet.hpp
    ${SRCROOT}/Socket.cpp
    ${INCROOT}/Socket.hpp
    ${SRCROOT}/SocketImpl.hpp
    ${INCROOT}/SocketHandle.hpp
    ${SRCROOT}/SocketSelector.cpp
    ${INCROOT}/SocketSelector.hpp
    ${SRCROOT}/TcpListener.cpp
    ${INCROOT}/TcpListener.hpp
    ${SRCROOT}/TcpSocket.cpp
    ${INCROOT}/TcpSocket.hpp
    ${SRCROOT}/UdpSocket.cpp
    ${INCROOT}/UdpSocket.hpp
)

# add platform specific sources
if(SFML_OS_WINDOWS)
    list(APPEND SRC
        ${SRCROOT}/Win32/SocketImpl.cpp
    )
else()
    list(APPEND SRC
        ${SRCROOT}/Unix/SocketImpl.cpp
    )
endif()

source_group("" FILES ${SRC})

# define the sfml-network target
sfml_add_library(Network
                 SOURCES ${SRC})

# setup dependencies
target_link_libraries(sfml-network PUBLIC SFML::System)
if(SFML_OS_WINDOWS)
    target_link_libraries(sfml-network PRIVATE ws2_32)
endif()

# Add MongoDB dependencies
target_link_libraries(sfml-network PRIVATE mongo::bson_shared mongo::mongoc_shared)
target_include_directories(sfml-network PRIVATE ${BSON_INCLUDE_DIRS} ${MONGOC_INCLUDE_DIRS})
