cmake_minimum_required(VERSION 3.15)
project(BLADE VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Set Qt path - adjust this if Qt is installed elsewhere
if(NOT DEFINED CMAKE_PREFIX_PATH)
    set(CMAKE_PREFIX_PATH "C:/Qt/6.8.1/mingw_64")
endif()

message(STATUS "Looking for Qt at: ${CMAKE_PREFIX_PATH}")

if(MSVC)
    add_compile_options(/W4)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
else()
    add_compile_options(-Wall -Wextra -pedantic)
    # Static linking for MinGW
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libgcc -static-libstdc++ -pthread")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libgcc -static-libstdc++")
endif()

find_package(Qt6 REQUIRED COMPONENTS Core Gui Network Qml Quick Svg)

if(Qt6_FOUND)
    message(STATUS "Qt6 found successfully!")
    message(STATUS "Qt6 version: ${Qt6_VERSION}")
else()
    message(FATAL_ERROR "Qt6 not found! Please install Qt6 or set CMAKE_PREFIX_PATH to your Qt installation.")
endif()

include_directories(${PROJECT_SOURCE_DIR}/include)

set(SOURCES
    src/main.cpp
    src/Application.cpp
    src/Server.cpp
    src/AuthenticationManager.cpp
    src/ConnectionHandler.cpp
    src/HTTPServer.cpp
    src/NetworkUtils.cpp
    src/QRCodeGen.cpp
    src/Logger.cpp
)

set(HEADERS
    include/Application.h
    include/Server.h
    include/AuthenticationManager.h
    include/ConnectionHandler.h
    include/HTTPServer.h
    include/NetworkUtils.h
    include/QRCodeGen.h
    include/Logger.h
)

set(RESOURCES
    resources.qrc
)

add_executable(blade ${SOURCES} ${HEADERS} ${RESOURCES})

target_link_libraries(blade PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Network
    Qt6::Qml
    Qt6::Quick
    Qt6::Svg
)

if(WIN32)
    if(MINGW)
        set(MANIFEST_RC "${CMAKE_CURRENT_BINARY_DIR}/blade.rc")
        file(WRITE ${MANIFEST_RC} "IDI_ICON1 ICON \"${CMAKE_SOURCE_DIR}/blade.ico\"\n1 24 \"${CMAKE_SOURCE_DIR}/blade.manifest\"\n")
        target_sources(blade PRIVATE ${MANIFEST_RC})
        # Hide console window using -mwindows
        target_link_options(blade PRIVATE -mwindows)
    elseif(MSVC)
        set(MANIFEST_RC "${CMAKE_CURRENT_BINARY_DIR}/blade.rc")
        file(WRITE ${MANIFEST_RC} "IDI_ICON1 ICON \"${CMAKE_SOURCE_DIR}/blade.ico\"\n")
        target_sources(blade PRIVATE ${MANIFEST_RC})
        set_target_properties(blade PROPERTIES
            LINK_FLAGS "/MANIFESTUAC:\"level='requireAdministrator' uiAccess='false'\" /SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup"
        )
    endif()
endif()

find_package(Threads REQUIRED)
target_link_libraries(blade PRIVATE Threads::Threads)

if(WIN32)
    target_link_libraries(blade PRIVATE ws2_32 secur32 crypt32 iphlpapi)
elseif(UNIX)
    target_link_libraries(blade PRIVATE pthread)
endif()

file(MAKE_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

# Copy blade.ico and web directory to build directory for development/debugging
add_custom_command(TARGET blade POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_SOURCE_DIR}/blade.ico
        ${CMAKE_CURRENT_BINARY_DIR}/blade.ico
    COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_SOURCE_DIR}/styles.qss
        ${CMAKE_CURRENT_BINARY_DIR}/styles.qss
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/web
        ${CMAKE_CURRENT_BINARY_DIR}/web
    COMMENT "Copying blade.ico, styles.qss, and web directory to build folder..."
)

add_custom_command(TARGET blade POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        $<TARGET_FILE:blade>
        ${CMAKE_SOURCE_DIR}/bin/blade.exe
    COMMENT "Copying blade.exe to bin folder..."
)

# Deploy Qt dependencies
if(WIN32)
    find_program(WINDEPLOYQT_EXECUTABLE windeployqt HINTS "${CMAKE_PREFIX_PATH}/bin")
    if(WINDEPLOYQT_EXECUTABLE)
        add_custom_command(TARGET blade POST_BUILD
            COMMAND ${WINDEPLOYQT_EXECUTABLE} --no-translations "${CMAKE_SOURCE_DIR}/bin/blade.exe"
            COMMENT "Deploying Qt dependencies to bin folder..."
        )
    else()
        message(WARNING "windeployqt not found. Qt DLLs will not be automatically copied.")
    endif()

    # Copy MinGW runtime DLLs
    if(MINGW)
        set(MINGW_BIN_DIR "${CMAKE_PREFIX_PATH}/bin")
        add_custom_command(TARGET blade POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${MINGW_BIN_DIR}/libwinpthread-1.dll"
                "${CMAKE_SOURCE_DIR}/bin/"
            COMMENT "Copying MinGW pthread DLL to bin folder..."
        )
    endif()
endif()

add_custom_command(TARGET blade POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_SOURCE_DIR}/blade.ico
        ${CMAKE_SOURCE_DIR}/bin/blade.ico
    COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_SOURCE_DIR}/styles.qss
        ${CMAKE_SOURCE_DIR}/bin/styles.qss
    COMMENT "Copying blade.ico and styles.qss to bin folder..."
)

add_custom_command(TARGET blade POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/web
        ${CMAKE_SOURCE_DIR}/bin/web
    COMMENT "Copying web directory to bin folder..."
)

add_custom_command(TARGET blade POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_SOURCE_DIR}/blade.ico
        ${CMAKE_SOURCE_DIR}/bin/web/blade.ico
    COMMENT "Copying blade.ico to bin/web folder..."
)

install(TARGETS blade DESTINATION bin)
install(DIRECTORY web/ DESTINATION share/blade/web)

