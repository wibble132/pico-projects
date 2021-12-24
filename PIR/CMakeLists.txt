
add_executable(pirTest pirTest.c)

target_link_libraries(pirTest pico_stdlib)

pico_add_extra_outputs(pirTest)
