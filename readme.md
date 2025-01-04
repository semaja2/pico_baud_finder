# pico baud finder

## Overview
This project is an attempt to utilise a RP2040 board (Raspberry Pico W) to analyse pulse times on a UART pin and calculate a probably baud rate.

Once a baud is found it is possible to switch to a terminal mode which will set the `Serial1` port to the detected baud, and redirect the input/outputs between `Serial` (USB) and `Serial1`.

## Why
It seemed like a fun problem to solve, and removes the need for a logic analyser or brute forcing baud rates when investigating UART on unknown devices.

Credit to AMAL for the original Arduino code and concept (https://www.electronicsforu.com/electronics-projects/hardware-diy/uart-automatic-baud-rate-detector)

## How to use
Upon powering the device it will run in detect mode, monitoring the serial output over USB will show calculated baud rates.

Once a baud rate is found you can press the `BOOTSEL` button to switch to terminal mode, the LED will turn on to indicate this mode.

To return to the detect mode simply press the `BOOTSEL` button again, the LED will turn off to indicate it is in detect mode.

## Observed Baud Rates
The below rates were observed from collecting the rates measured during known baud rate transmissions.

| Baud   | Rate Range  |
| ------ | ----------- |
| 300    | 3333        |
| 600    | 1666-1667   |
| 1200   | 832-834     |
| 2400   | 415-417     |
| 3600   | 274-278     |
| 4800   | 206-209     |
| 7200   | 138-139     |
| 9600   | 103-104     |
| 14400  | 65-70       |
| 19200  | 50-52       |
| 28800  | 30-40       |
| 38400  | 25-26       |
| 57600  | 16-17       |
| 115200 | 6-9         |
| 230400 | 3-5         |
| 460800 | Unstable    |

## Known issues
In terminal mode it observes frequent noise/garbage resulting, it is best to use detect mode to find baud and then switch to a proper UART adapter until resolved.

## Future ideas
 - Reimplement combined mode with detection occuring on core1 and terminal occuring on core2, allowing for simpler operation