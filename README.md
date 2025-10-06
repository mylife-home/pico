# pico
RaspberryPI Pico firmwares

## Build deps
- ~workspace/pico-sdk
- build ~/workspace/pico_i2c_slave

## Build

- mkdir build (or rm -rf build/*)
- cd build
- PICO_SDK_PATH=~/workspace/pico-sdk cmake ..
- make -j4
- pick .uf2 file and move it to rpi dir

## Debug (OSX)

- Connect usb port
- ls -lsah /dev/tty.usbmodem*
- screen /dev/tty.usbmodem1101
  - help