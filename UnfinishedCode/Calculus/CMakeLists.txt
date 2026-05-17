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

# add_subdirectory(Libs/qcoro)

include_directories(
    "${CMAKE_CURRENT_SOURCE_DIR}/Solution/controller/include"
        "${CMAKE_CURRENT_SOURCE_DIR}/Solution/controller/include/Models"
        "${CMAKE_CURRENT_SOURCE_DIR}/Solution/controller/include/Models/Operators"
)

BuildRadicalQt6Solution(PrivateLibs PublicLibs)

file(COPY_FILE "${CMAKE_CURRENT_SOURCE_DIR}/.clang-format_21" "${CMAKE_CURRENT_SOURCE_DIR}/.clang-format")

# target_link_libraries(${THIS} PUBLIC QCoro6::Coro QCoro6::Core)
