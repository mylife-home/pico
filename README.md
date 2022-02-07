# pico
RaspberryPI Pico firmwares

## Build deps
- ~workspace/pico-sdk
- build ~/workspace/pico_i2c_slave

## Build

- mkdir build
- cd build
- PICO_SDK_PATH=~/workspace/pico-sdk cmake ..
- make -j4
- pick .uf2 file and move it to rpi dir