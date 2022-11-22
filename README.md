# Pico-CHIP8
A CHIP-8 Emulator for RP2040/Raspberry Pi Pico - Currently a work-in progress.

## Hardware
This project is based around the Raspberry Pi Pico (RP2040) microcontroller. 

The display being used is a 128x64 I2C OLED display based on the SSD1306 driver chip. CHIP-8's display resolution fortunately enough fits on the display at a perfect 2x resolution. SUPER-CHIP should fit at 1x, however SUPER-CHIP compatibility is outside of the scope of this project currently due to time restrictions. Maybe eventually!

## Compiling & Uploading
Compiling is very straight forward. [Make sure to have a pico-sdk environment set up](https://datasheets.raspberrypi.org/pico/getting-started-with-pico.pdf), `mkdir build && cd build`, then `cmake ..`. Build files should generate successfully if you have pico-sdk set up correctly. Then just `make -j${nproc}`.

To upload, plug in your RPi Pico while holding the BOOTSEC button, then drag the UF2 file in the build directory onto the newly mounted device. The pico will automatically unmount itself and start running the newly uploaded code.

## Footnotes
This project was created for the [BitBuilt 2022 Secret Santa](https://bitbuilt.net/forums/index.php?threads/bitbuilts-2022-secret-santa.5384/) - Whoever got the original hardware I build for this, hope you enjoy it!