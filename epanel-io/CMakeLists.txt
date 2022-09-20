function(build_epanel_io i2c_address)
  set(executable_name "epanel-io-${i2c_address}")

  add_executable("${executable_name}"
    state.cc
    com.cc
    main.cc
    zc.cc
    dimmer.cc
    button.cc
  )

  target_compile_definitions("${executable_name}" PUBLIC I2C_ADDRESS=0x0${i2c_address})

  pico_enable_stdio_usb("${executable_name}" 1)
  pico_enable_stdio_uart("${executable_name}" 0)
  pico_add_extra_outputs("${executable_name}")
  target_link_libraries("${executable_name}" pico_stdlib hardware_adc hardware_i2c i2c_slave common)
endfunction()

build_epanel_io(1)
build_epanel_io(2)

