set(INCROOT "${SFML_SOURCE_DIR}/include")
set(SRCROOT "${SFML_SOURCE_DIR}/src")

# Find required packages
find_package(bson-1.0 REQUIRED)
find_package(mongoc-1.0 REQUIRED)

# Find MySQL using pkg-config
find_package(PkgConfig REQUIRED)
pkg_check_modules(MYSQL REQUIRED mysqlclient)

# List all source files
set(SFML_NETWORK_SOURCES
    ${SRCROOT}/SFML/Network/Ftp.cpp
    ${SRCROOT}/SFML/Network/Http.cpp
    ${SRCROOT}/SFML/Network/IpAddress.cpp
    ${SRCROOT}/SFML/Network/Packet.cpp
    ${SRCROOT}/SFML/Network/Socket.cpp
    ${SRCROOT}/SFML/Network/SocketSelector.cpp
    ${SRCROOT}/SFML/Network/TcpListener.cpp
    ${SRCROOT}/SFML/Network/TcpSocket.cpp
    ${SRCROOT}/SFML/Network/UdpSocket.cpp
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
        sfml-system
    PRIVATE
        mongo::bson_shared
        mongo::mongoc_shared
        ${MYSQL_LIBRARIES}
)
