cmake_minimum_required(VERSION 3.22)

#
# This file is generated only once,
# and is not re-generated if converter is called multiple times.
#
# User is free to modify the file as much as necessary
#

# Setup compiler settings
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


# Define the build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

# Set the project name
set(CMAKE_PROJECT_NAME 1_MotionControllerFirmware_IBNRoboter)

# Enable compile command to ease indexing with e.g. clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

# Core project settings
project(${CMAKE_PROJECT_NAME})
message("Build type: " ${CMAKE_BUILD_TYPE})

# Enable CMake support for ASM and C languages
enable_language(C CXX ASM)

# Create an executable object type
add_executable(${CMAKE_PROJECT_NAME})

# Add STM32CubeMX generated sources
add_subdirectory(cmake/stm32cubemx)

target_link_directories(${CMAKE_PROJECT_NAME} PRIVATE
)

# --- NEU: alle eigenen .cpp-Dateien automatisch einsammeln ---
file(GLOB_RECURSE APP_CXX_SOURCES CONFIGURE_DEPENDS
    ${CMAKE_SOURCE_DIR}/Core/Src/Src_Application/*.cpp
)

# CANopenNode (aus dem vorherigen Schritt)
file(GLOB_RECURSE CANOPEN_SOURCES CONFIGURE_DEPENDS
    ${CMAKE_SOURCE_DIR}/CANopen/CANopenNode/*.c
    ${CMAKE_SOURCE_DIR}/CANopen/CANopenNode_STM32/*.c
)

target_sources(${CMAKE_PROJECT_NAME} PRIVATE
    ${APP_CXX_SOURCES}
    ${CANOPEN_SOURCES}
)

file(GLOB_RECURSE CANOPEN_HEADERS CONFIGURE_DEPENDS
    ${CMAKE_SOURCE_DIR}/CANopen/CANopenNode/*.h
    ${CMAKE_SOURCE_DIR}/CANopen/CANopenNode_STM32/*.h
)
set(CANOPEN_INCLUDE_DIRS "")
foreach(HEADER ${CANOPEN_HEADERS})
    get_filename_component(DIR ${HEADER} DIRECTORY)
    list(APPEND CANOPEN_INCLUDE_DIRS ${DIR})
endforeach()
list(REMOVE_DUPLICATES CANOPEN_INCLUDE_DIRS)

target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    ${CANOPEN_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/Core/Inc
    ${CMAKE_SOURCE_DIR}/Core/Inc/Inc_Application
)

target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE
)

list(REMOVE_ITEM CMAKE_C_IMPLICIT_LINK_LIBRARIES ob)

target_link_libraries(${CMAKE_PROJECT_NAME}
stm32cubemx
)
