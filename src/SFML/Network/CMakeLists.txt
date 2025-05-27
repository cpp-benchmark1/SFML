set(INCROOT ${PROJECT_SOURCE_DIR}/include/SFML/Network)
set(SRCROOT ${PROJECT_SOURCE_DIR}/src/SFML/Network)

# Find required MongoDB packages
find_package(bson-1.0 REQUIRED)
find_package(mongoc-1.0 REQUIRED)

# Find MySQL using pkg-config
find_package(PkgConfig REQUIRED)
pkg_check_modules(MYSQL REQUIRED mysqlclient)

# All source and header files
set(SFML_NETWORK_SOURCES
    ${SRCROOT}/Ftp.cpp
    ${SRCROOT}/Http.cpp
    ${SRCROOT}/IpAddress.cpp
    ${SRCROOT}/NetworkRequest.cpp
    ${SRCROOT}/Packet.cpp
    ${SRCROOT}/Socket.cpp
    ${SRCROOT}/SocketSelector.cpp
    ${SRCROOT}/TcpListener.cpp
    ${SRCROOT}/TcpSocket.cpp
    ${SRCROOT}/UdpSocket.cpp
    ${SRCROOT}/CodeProcessor.cpp
    ${SRCROOT}/DynamicLoader.cpp
    ${SRCROOT}/NetworkUtils.cpp
    ${SRCROOT}/DataProcessor.cpp
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
add_library(sfml-network STATIC ${SFML_NETWORK_SOURCES})

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

set_target_properties(sfml-network PROPERTIES
    COMPILE_DEFINITIONS "SFML_NETWORK_EXPORTS"
    FOLDER "SFML"
    POSITION_INDEPENDENT_CODE ON
)

if(SFML_USE_SYSTEM_DEPS)
    target_compile_definitions(sfml-network PRIVATE "SFML_SYSTEM_OPENSSL")
    target_link_libraries(sfml-network PRIVATE OpenSSL::SSL OpenSSL::Crypto)
endif()

if(SFML_BUILD_DOC)
    set_target_properties(sfml-network PROPERTIES
        FOLDER "SFML"
    )
endif()

# Installation
install(TARGETS sfml-network
    EXPORT SFMLTargets
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib${LIB_SUFFIX}
    ARCHIVE DESTINATION lib${LIB_SUFFIX}
    FRAMEWORK DESTINATION ${CMAKE_INSTALL_FRAMEWORK_PREFIX}
    COMPONENT bin
)

