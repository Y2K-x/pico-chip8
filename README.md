# Pico-CHIP8
A CHIP-8 Emulator for RP2040/Raspberry Pi Pico

## Hardware
This project is based around the Raspberry Pi Pico (RP2040) microcontroller. 

The display being used is a 128x64 I2C OLED display based on the SSD1306 driver chip. CHIP-8's display resolution fortunately enough fits on the display at a perfect 2x resolution. SUPER-CHIP should fit at 1x, however SUPER-CHIP compatibility is outside of the scope of this project currently due to time restrictions. Maybe eventually!

## Compiling & Uploading
Compiling is very straight forward. [Make sure to have a pico-sdk environment set up](https://datasheets.raspberrypi.org/pico/getting-started-with-pico.pdf), 
clone the repo with `--recursive`, `mkdir build && cd build`, then `cmake ..`. Build files should generate successfully if you have pico-sdk set up correctly. Then just `make -j${nproc}`.

To upload, plug in your RPi Pico while holding the BOOTSEC button, then drag the UF2 file in the build directory onto the newly mounted device. The pico will automatically unmount itself and start running the newly uploaded code.

## Wiring
~ Input ~

Wiring is fairly straightforward. For button input you'll want a 4x4 button matrix. Those membrane keypads might work but I have not tried them, but otherwise, each row of buttons has one end of the switch wired together, and each collumn has the other end wired together.

GP0 through GP3 - row 1 through row 4
GP4 through GP7 - col 4 through col 1 (yes, backwards, made more sense when wiring my unit up)

~ Display ~

MOSI - GP19

CS - GP16

SCLK - GP18

RST - GP20

DC - GP17

~ SD module~

Any micro sd module should work as long as it has card detect pins. If yours does not have it broken out on the PCB, you may have to solder to 2 pins on the slot itself. One goes to GND, the other to pin 14.

MOSI - GP11

MISO - GP12

SCLK - GP10

CS - GP13

Card Detect - GP14

~ Audio ~

Audio is just a simple 3v3 beeper speaker you can find on Amazon. Wire the positive end to pin 8. Negative end goes to ground.

## Footnotes
This project was created for the [BitBuilt 2022 Secret Santa](https://bitbuilt.net/forums/index.php?threads/bitbuilts-2022-secret-santa.5384/) - Whoever got the original hardware I build for this, hope you enjoy it!