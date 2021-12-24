
add_executable(lcd_uart lcd_uart.c)

pico_generate_pio_header(lcd_uart ${CMAKE_CURRENT_LIST_DIR}/lcd_uart.pio)

target_link_libraries(lcd_uart pico_stdlib hardware_pio)

pico_add_extra_outputs(lcd_uart)

pico_enable_stdio_usb(lcd_uart 1)
pico_enable_stdio_uart(lcd_uart 0)