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

set_property(SOURCE ${APP_CXX_SOURCES} APPEND PROPERTY COMPILE_OPTIONS "-Wdouble-promotion")

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

# --- Auswahl der Platinenvariante (CONTROLLER_1 / CONTROLLER_2 / CONTROLLER_3) ---
# BOARD ist eine CMake-Variable (Standard: 1) und bestimmt das Praeprozessor-Define
# CONTROLLER_<BOARD>. Komfortabel ueber die Presets Board1/Board2/Board3 in
# CMakePresets.json waehlbar (baut in build/Board<N>).
set(BOARD 1 CACHE STRING "Platinenvariante: 1, 2 oder 3 (setzt CONTROLLER_<BOARD>)")
if(NOT BOARD MATCHES "^[123]$")
    message(FATAL_ERROR "BOARD muss 1, 2 oder 3 sein (ist: '${BOARD}')")
endif()
message("Platinenvariante: " ${BOARD} " -> CONTROLLER_" ${BOARD})

target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE
    CONTROLLER_${BOARD}
)

list(REMOVE_ITEM CMAKE_C_IMPLICIT_LINK_LIBRARIES ob)

target_link_libraries(${CMAKE_PROJECT_NAME}
stm32cubemx
)

# --- Fehlerbehebung für ARM/Thumb Linker-Konflikt ---
target_compile_options(${CMAKE_PROJECT_NAME} PRIVATE 
    $<$<COMPILE_LANGUAGE:CXX>:-mthumb -mthumb-interwork>
    $<$<COMPILE_LANGUAGE:C>:-mthumb -mthumb-interwork>
)

target_link_options(${CMAKE_PROJECT_NAME} PRIVATE 
    -mthumb 
    -mthumb-interwork
)
