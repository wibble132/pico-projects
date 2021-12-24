
add_executable(adcTest adcTest.c)

# pull in common dependencies
target_link_libraries(adcTest pico_stdlib hardware_adc)

#create map/bin/hex file etc.
pico_add_extra_outputs(adcTest)

# test1, printing the value
pico_enable_stdio_usb(adcTest 1)
pico_enable_stdio_uart(adcTest 0)