cmake_minimum_required(VERSION 3.15)
project(BLADE VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 23)
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
    src/QRCodeGen.cpp
)

# Header files
set(HEADERS
    include/Server.h
    include/AuthenticationManager.h
    include/ConnectionHandler.h
    include/HTTPServer.h
    include/NetworkUtils.h
    include/QRCodeGen.h
)

# Create executable
add_executable(blade ${SOURCES} ${HEADERS})

# Windows: Embed manifest and icon
if(WIN32)
    if(MINGW)
        # For MinGW, create a resource file to embed the manifest and icon
        set(MANIFEST_RC "${CMAKE_CURRENT_BINARY_DIR}/blade.rc")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libgcc -static-libstdc++ -static")
        file(WRITE ${MANIFEST_RC} "IDI_ICON1 ICON \"${CMAKE_SOURCE_DIR}/blade.ico\"\n1 24 \"${CMAKE_SOURCE_DIR}/blade.manifest\"\n")

        # Add the RC file to the target
        target_sources(blade PRIVATE ${MANIFEST_RC})
    elseif(MSVC)
        # For MSVC, create a resource file for icon and use linker flags for manifest
        set(MANIFEST_RC "${CMAKE_CURRENT_BINARY_DIR}/blade.rc")
        file(WRITE ${MANIFEST_RC} "IDI_ICON1 ICON \"${CMAKE_SOURCE_DIR}/blade.ico\"\n")
        target_sources(blade PRIVATE ${MANIFEST_RC})

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
    target_link_libraries(blade PRIVATE ws2_32 secur32 crypt32 iphlpapi)
elseif(UNIX)
    target_link_libraries(blade PRIVATE pthread)
endif()

# Create bin directory if it doesn't exist
file(MAKE_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

# Copy blade.exe to bin folder after successful build
add_custom_command(TARGET blade POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        $<TARGET_FILE:blade>
        ${CMAKE_SOURCE_DIR}/bin/blade.exe
    COMMENT "Copying blade.exe to bin folder..."
)

# Copy blade.ico to bin folder for web interface
add_custom_command(TARGET blade POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_SOURCE_DIR}/blade.ico
        ${CMAKE_SOURCE_DIR}/bin/blade.ico
    COMMENT "Copying blade.ico to bin folder..."
)

# Copy web directory to bin folder after successful build
add_custom_command(TARGET blade POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/web
        ${CMAKE_SOURCE_DIR}/bin/web
    COMMENT "Copying web directory to bin folder..."
)

# Copy blade.ico to bin/web folder for web interface
add_custom_command(TARGET blade POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_SOURCE_DIR}/blade.ico
        ${CMAKE_SOURCE_DIR}/bin/web/blade.ico
    COMMENT "Copying blade.ico to bin/web folder..."
)

# Installation
install(TARGETS blade DESTINATION bin)
install(DIRECTORY web/ DESTINATION share/blade/web)
