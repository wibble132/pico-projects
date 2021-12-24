
add_executable(ws2812)

pico_generate_pio_header(ws2812 ${CMAKE_CURRENT_LIST_DIR}/ws2812.pio)

target_sources(ws2812 PRIVATE ws2812.c)

target_link_libraries(ws2812 PRIVATE
        pico_stdlib
        hardware_pio
        hardware_adc
        )

pico_add_extra_outputs(ws2812)

pico_enable_stdio_usb(ws2812 1)
pico_enable_stdio_uart(ws2812 0)