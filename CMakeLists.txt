cmake_minimum_required(VERSION 3.12)

include(pico_sdk_import.cmake)

project(mypico C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

if (PICO_SDK_VERSION_STRING VERSION_LESS "1.3.0")
    message(FATAL_ERROR "Raspberry Pi Pico DSK cersion 1.3.0 (or later) required. Your version is ${PICO_SDK_VERSION_STRING}")
endif()

pico_sdk_init()


add_compile_options(-Wall
        -Wno-format
        -Wno-unused-function
        -Wno-maybe-unitialized)
     
add_subdirectory(blink)
add_subdirectory(adcTest)
add_subdirectory(PIO)
add_subdirectory(PIR)
add_subdirectory(AutoLightStrip)
add_subdirectory(dma)
add_subdirectory(lcd_uart)
