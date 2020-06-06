# ArduinoADB
ADB to USB converter using an Arduino Uno (others may work as well).

## Connections
The ADB connection should be set up with the 5V pin to 5V, ground to ground, data to pin 8, and a 1-10K resistor between 5V and pin 8. Here's a visual representation for reference:
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
|        |     |
GND     Pin8  5V
```
Note: If you don't have a female 4-pin Mini-DIN port, you can substitute with three female jumper wires connecting to the pins on the cable, or if using an Extended Keyboard, you can use three male jumper wires connecting to the pins. If connecting to the wire, make sure to swap the pin layout horizontally.

## Setup
1. Upload the sketch to your Arduino.
2. Put your Arduino into DFU mode by shorting the two pins nearest to the reset button (they will be on the 6-pin header nearest to the reset button).
3. Open Atmel FLIP or dfu-programmer and flash the `Arduino-keyboard-0.3.hex` firmware to the Arduino.
4. Disconnect and reconnect the Arduino to the computer.

## Usage
The Arduino itself is Plug & Play, but do not unplug the keyboard from the Arduino. I cannot confirm whether the ADB port here is hot-pluggable, but assume it isn't.
