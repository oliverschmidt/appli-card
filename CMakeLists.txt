set(PROJECT_NAME Appli-Card)

cmake_minimum_required(VERSION 3.13...3.27)
include(pico_sdk_import.cmake)
project(${PROJECT_NAME} C CXX ASM)
pico_sdk_init()

add_executable(${PROJECT_NAME})
pico_add_extra_outputs(${PROJECT_NAME})

option(DEBUG "Debug output via USB CDC-ACM")
message("DEBUG is ${DEBUG}")

pico_enable_stdio_uart(${PROJECT_NAME} 0)
if (DEBUG)
        pico_enable_stdio_usb(${PROJECT_NAME} 1)
endif ()

set_source_files_properties(incbin.S
        OBJECT_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/applicard-v9.bin
        )

include(FetchContent)
FetchContent_Declare(a2pico
        GIT_REPOSITORY https://github.com/oliverschmidt/a2pico.git
        GIT_TAG main
        )
FetchContent_MakeAvailable(a2pico)

if (DEBUG)
        target_compile_definitions(${PROJECT_NAME} PRIVATE DEBUG)
endif ()

target_sources(${PROJECT_NAME} PRIVATE
        board.c
        incbin.S
        main.c
        z80.c
        )

target_link_libraries(${PROJECT_NAME} PRIVATE
        a2pico
        pico_multicore
        pico_stdlib
        )
