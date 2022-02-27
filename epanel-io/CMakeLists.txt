add_executable(epanel-io
  state.cc
  com.cc
  main.cc
  zc.cc
)

pico_enable_stdio_usb(epanel-io 1)
pico_enable_stdio_uart(epanel-io 0)
pico_add_extra_outputs(epanel-io)
target_link_libraries(epanel-io pico_stdlib hardware_i2c i2c_slave common)