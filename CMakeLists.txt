cmake_minimum_required(VERSION 3.15)
project(BLADE VERSION 1.0.0 LANGUAGES CXX)

# NOTE: Compiler selection is handled via CMake toolchain file or IDE settings.
# Do NOT set CMAKE_C_COMPILER / CMAKE_CXX_COMPILER here - it's too late after project().
# Use cmake/toolchain-qt-mingw.cmake or configure your IDE's CMake profile.

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
    # NOTE: Do NOT force static libstdc++/libgcc for Qt on Windows/MinGW.
    # It commonly causes runtime crashes / missing entry points.
endif()

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Network Svg)

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
    src/MainWindow.cpp
    src/LoginWidget.cpp
    src/ServerWidget.cpp
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
    include/MainWindow.h
    include/LoginWidget.h
    include/ServerWidget.h
    include/Server.h
    include/AuthenticationManager.h
    include/ConnectionHandler.h
    include/HTTPServer.h
    include/NetworkUtils.h
    include/QRCodeGen.h
    include/Logger.h
    include/TitleBar.h
    include/Toast.h
)

set(RESOURCES
    resources.qrc
)

# Build as a GUI app on Windows.
# For MinGW, using WIN32 may pull in Qt6EntryPoint which can fail to link depending on CRT.
# Instead, build a normal executable and set the Windows subsystem explicitly.
if(WIN32)
    add_executable(blade ${SOURCES} ${HEADERS} ${RESOURCES})
    if(MINGW)
        target_link_options(blade PRIVATE "-mwindows")
    endif()
else()
    add_executable(blade ${SOURCES} ${HEADERS} ${RESOURCES})
endif()

target_link_libraries(blade PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Network
    Qt6::Svg
)

if(WIN32)
    if(MINGW)
        set(MANIFEST_RC "${CMAKE_CURRENT_BINARY_DIR}/blade.rc")
        file(WRITE ${MANIFEST_RC} "IDI_ICON1 ICON \"${CMAKE_SOURCE_DIR}/blade.ico\"\n1 24 \"${CMAKE_SOURCE_DIR}/blade.manifest\"\n")
        target_sources(blade PRIVATE ${MANIFEST_RC})
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

# ---- Runtime layout / deployment ----
set(DEPLOY_DIR "${CMAKE_SOURCE_DIR}/bin")
file(MAKE_DIRECTORY "${DEPLOY_DIR}")

# Copy assets to build folder (dev) and to bin (runtime)
add_custom_command(TARGET blade POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/blade.ico"
        "$<TARGET_FILE_DIR:blade>/blade.ico"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/styles-widgets.qss"
        "$<TARGET_FILE_DIR:blade>/styles-widgets.qss"
    COMMENT "Copying runtime assets to build output folder..."
)

# Copy built exe into bin
add_custom_command(TARGET blade POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "$<TARGET_FILE:blade>"
        "${DEPLOY_DIR}/$<TARGET_FILE_NAME:blade>"
    COMMENT "Copying blade executable to bin folder..."
)

# Deploy Qt dependencies.
# IMPORTANT: run windeployqt on the *built* executable, then copy the produced DLLs/plugins to bin.
if(WIN32)
    find_program(WINDEPLOYQT_EXECUTABLE windeployqt HINTS "${CMAKE_PREFIX_PATH}/bin")
    if(WINDEPLOYQT_EXECUTABLE)
        add_custom_command(TARGET blade POST_BUILD
            COMMAND "${WINDEPLOYQT_EXECUTABLE}"
                --no-translations
                --no-system-d3d-compiler
                --no-opengl-sw
                "$<TARGET_FILE:blade>"
            COMMAND ${CMAKE_COMMAND} -E copy_directory
                "$<TARGET_FILE_DIR:blade>"
                "${DEPLOY_DIR}"
            COMMENT "Deploying Qt dependencies and copying runtime files to bin..."
        )

        # Also copy the MinGW runtime DLLs from the compiler's bin directory.
        # This avoids 'Entry Point Not Found' crashes when multiple MinGW toolchains exist.
        # Get the directory containing the C++ compiler
        get_filename_component(MINGW_BIN_PATH "${CMAKE_CXX_COMPILER}" DIRECTORY)
        message(STATUS "MinGW runtime DLLs will be copied from: ${MINGW_BIN_PATH}")

        add_custom_command(TARGET blade POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${MINGW_BIN_PATH}/libstdc++-6.dll"
                "${DEPLOY_DIR}/libstdc++-6.dll"
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${MINGW_BIN_PATH}/libgcc_s_seh-1.dll"
                "${DEPLOY_DIR}/libgcc_s_seh-1.dll"
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${MINGW_BIN_PATH}/libwinpthread-1.dll"
                "${DEPLOY_DIR}/libwinpthread-1.dll"
            COMMENT "Copying MinGW runtime DLLs from compiler directory to bin..."
        )
    else()
        message(WARNING "windeployqt not found. Qt DLLs will not be automatically copied.")
    endif()
endif()

# Copy assets into bin (after deploy) - keep it simple and non-destructive.
add_custom_command(TARGET blade POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/blade.ico"
        "${DEPLOY_DIR}/blade.ico"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/styles-widgets.qss"
        "${DEPLOY_DIR}/styles-widgets.qss"
    COMMENT "Copying assets to bin folder..."
)

install(TARGETS blade DESTINATION bin)
install(DIRECTORY web/ DESTINATION share/blade/web)
