

add_executable(auto_light_strip autoLightStrip.c)

pico_generate_pio_header(auto_light_strip ${CMAKE_CURRENT_LIST_DIR}/autoLightStrip.pio )

target_link_libraries(auto_light_strip pico_stdlib hardware_pio hardware_dma hardware_adc)

pico_add_extra_outputs(auto_light_strip)

# pico_enable_stdio_usb(auto_light_strip true)
# pico_enable_stdio_uart(auto_light_strip false)
