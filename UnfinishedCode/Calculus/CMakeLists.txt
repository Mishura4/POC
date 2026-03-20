cmake_minimum_required(VERSION 3.17)

if(WIN32)
    set(RADICAL_PATH "C:/Source/CMake/Modules")
else()
    set(RADICAL_PATH "/opt/Radicalware/CMake_Modules")
endif()
include("${RADICAL_PATH}/Radical-App-Build-Qt6.cmake")

if(NOT DEFINED ArgProjectName OR ArgProjectName STREQUAL "")
    get_filename_component(ArgProjectName "${CMAKE_SOURCE_DIR}" NAME)
endif()

set(THIS ${ArgProjectName})
project(${THIS} LANGUAGES CXX)

set(PrivateLibs "")
list(APPEND PrivateLibs "Iterator")
list(APPEND PrivateLibs "Memory")
list(APPEND PrivateLibs "Nexus")
list(APPEND PrivateLibs "xvector")
list(APPEND PrivateLibs "xstring")
list(APPEND PrivateLibs "Macros")
list(APPEND PrivateLibs "xmap")

set(PublicLibs "")
list(APPEND PublicLibs "OS")
list(APPEND PublicLibs "SYS")
list(APPEND PublicLibs "Timer")

add_compile_definitions(QT_QML_DEBUG)

BuildRadicalQt6Solution(PrivateLibs PublicLibs)
