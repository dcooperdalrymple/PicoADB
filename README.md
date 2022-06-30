# PicoADB
ADB to USB keyboard converter using a Raspberry Pi Pico or compatible RP2040 board.

## Connections
The ADB connection should be set up with the 5V pin to VBUS, ground to GND, a 1-10K resistor between VBUS and data, and data to GP2 through a 5V-to-3.3V bidirectional level shifter. Here's a visual representation for reference:
```
Female Mini-DIN 4
    /-----\
|====*4 3*=====|
| | *2   1*==| |
|  \  ===  / | |
|   \-----/  | |
|            | |
|        |===| |
|        |     |
|        |=[R]=|
|        |     |
|      SHIFT   |
|        |     |
GND      GP2   5V/VBUS
```
Note: If you don't have a female 4-pin Mini-DIN port, you can substitute with three female jumper wires connecting to the pins on the cable, or if using an Extended Keyboard, you can use three male jumper wires connecting to the pins. If connecting to the wire, make sure to swap the pin layout horizontally.

## Compiling Firmware

* Configuring: `cmake -B build -S .`
* Compiling/Building: `make -C build`
* Writing: Hold BOOTSEL button on Pico, plug it in via USB, and release BOOTSEL. Copy and paste `picoadb.uf2` into RPI-RP2 drive.

## Usage
The ADB port implementation is not currently hot-pluggable. Make sure to keep it connected during usage.

### Note on Caps Lock
Some keyboards have locking Caps Lock keys, including the Apple Extended Keyboard I/II and AppleDesign keyboard. For the Caps Lock key to work properly, you must define `LOCKING_CAPS` in the main sketch file. This is already done for you, but if you're using a keyboard that doesn't have a locking Caps Lock key, you must comment out that line.

### Serial Debug
If you need to debug the code for any reason, define `DEBUG` and recompile. 9600 baud UART messages should be sent on GP0. You'll need an external serial UART decoder to monitor messages on your computer.

## Original source
The code for communicating over ADB was borrowed from [tmk/tmk_keyboard](https://github.com/tmk/tmk_keyboard/blob/master/converter/adb_usb), and the implementation and notation was inspired by [ArduinoADB](https://github.com/MCJack123/ArduinoADB).
