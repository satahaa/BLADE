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

# Platform-specific libraries
if(WIN32)
    target_link_libraries(blade ws2_32)
elseif(UNIX)
    target_link_libraries(blade pthread)
endif()

# Installation
install(TARGETS blade DESTINATION bin)
install(DIRECTORY web/ DESTINATION share/blade/web)
