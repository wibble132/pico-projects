

add_executable(ws2812_multi ws2812_multi.c)

pico_generate_pio_header(ws2812_multi ${CMAKE_CURRENT_LIST_DIR}/ws2812_multi.pio)

target_link_libraries(ws2812_multi pico_stdlib hardware_pio hardware_dma)

pico_add_extra_outputs(ws2812_multi)

pico_enable_stdio_usb(ws2812_multi 1)
pico_enable_stdio_uart(ws2812_multi 0)