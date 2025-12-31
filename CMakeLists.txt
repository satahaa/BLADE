cmake_minimum_required(VERSION 3.15)
project(BLADE VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Compiler flags for optimization and warnings
if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -pedantic)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
endif()

# Include directories
include_directories(${PROJECT_SOURCE_DIR}/include)

# Source files
set(SOURCES
    src/main.cpp
    src/Server.cpp
    src/AuthenticationManager.cpp
    src/ConnectionHandler.cpp
    src/HTTPServer.cpp
    src/NetworkUtils.cpp
)

# Header files
set(HEADERS
    include/Server.h
    include/AuthenticationManager.h
    include/ConnectionHandler.h
    include/HTTPServer.h
    include/NetworkUtils.h
)

# Create executable
add_executable(blade ${SOURCES} ${HEADERS})

# Windows: Embed manifest to request administrator privileges for HTTPS
if(WIN32)
    if(MINGW)
        # For MinGW, create a resource file to embed the manifest
        set(MANIFEST_RC "${CMAKE_CURRENT_BINARY_DIR}/blade.rc")
        file(WRITE ${MANIFEST_RC} "1 24 \"${CMAKE_SOURCE_DIR}/blade.manifest\"\n")

        # Add the RC file to the target
        target_sources(blade PRIVATE ${MANIFEST_RC})
    elseif(MSVC)
        # For MSVC, use linker flags
        set_target_properties(blade PROPERTIES
            LINK_FLAGS "/MANIFESTUAC:\"level='requireAdministrator' uiAccess='false'\""
        )
    endif()
endif()

# Find and link threading library (required for std::thread, std::mutex)
find_package(Threads REQUIRED)
target_link_libraries(blade PRIVATE Threads::Threads)

# Platform-specific libraries
if(WIN32)
    target_link_libraries(blade PRIVATE ws2_32 secur32 crypt32)
elseif(UNIX)
    target_link_libraries(blade PRIVATE pthread)
endif()

# Installation
install(TARGETS blade DESTINATION bin)
install(DIRECTORY web/ DESTINATION share/blade/web)
