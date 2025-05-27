set(INCROOT ${PROJECT_SOURCE_DIR}/include/SFML/Network)
set(SRCROOT ${PROJECT_SOURCE_DIR}/src/SFML/Network)

# Find required MongoDB packages
find_package(bson-1.0 REQUIRED)
find_package(mongoc-1.0 REQUIRED)

# Find MySQL using pkg-config
find_package(PkgConfig REQUIRED)
pkg_check_modules(MYSQL REQUIRED mysqlclient)

# All source and header files
set(SRC
    ${INCROOT}/Export.hpp
    ${SRCROOT}/Ftp.cpp
    ${INCROOT}/Ftp.hpp
    ${SRCROOT}/Http.cpp
    ${INCROOT}/Http.hpp
    ${SRCROOT}/IpAddress.cpp
    ${INCROOT}/IpAddress.hpp
    ${SRCROOT}/NetworkRequest.cpp
    ${SRCROOT}/NetworkRequest.hpp
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


# Add platform-specific source files
if(SFML_SYSTEM_WINDOWS)
    list(APPEND SFML_NETWORK_SOURCES ${SRCROOT}/SFML/Network/Win32/SocketImpl.cpp)
elseif(SFML_SYSTEM_LINUX)
    list(APPEND SFML_NETWORK_SOURCES ${SRCROOT}/SFML/Network/Unix/SocketImpl.cpp)
elseif(SFML_SYSTEM_MACOS)
    list(APPEND SFML_NETWORK_SOURCES ${SRCROOT}/SFML/Network/Unix/SocketImpl.cpp)
endif()

# Create the Network module
add_library(sfml-network SHARED ${SFML_NETWORK_SOURCES})

# Define the SFML_NETWORK_EXPORTS macro
target_compile_definitions(sfml-network PRIVATE SFML_NETWORK_EXPORTS)

# Include directories
target_include_directories(sfml-network
    PUBLIC
        $<BUILD_INTERFACE:${INCROOT}>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${SRCROOT}
        ${MONGOC_INCLUDE_DIRS}
        ${BSON_INCLUDE_DIRS}
        ${MYSQL_INCLUDE_DIRS}
)

# Link dependencies
target_link_libraries(sfml-network
    PUBLIC
        SFML::System
    PRIVATE
        mongo::bson_shared
        mongo::mongoc_shared
        ${MYSQL_LIBRARIES}
)

# Link Windows socket library if on Windows
if(SFML_OS_WINDOWS)
    target_link_libraries(sfml-network PRIVATE ws2_32)
else()
    find_package(CURL REQUIRED)
    target_link_libraries(sfml-network PRIVATE CURL::libcurl)
endif()

