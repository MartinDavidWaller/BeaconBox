# BeaconBox - As published in Practical Wireless March 2025

BeaconBox - bringing life to the "YAESU THE RADIO AMATEUR's WORLD MAP"

<img src="Pictures/Fig5.jpg?sanitize=true&raw=true" />

## Modes

BeaconBox supports 3 three modes. The videos below demonstrate each one. 

### Beacons Heard Mode - Click on the image to launch the video.

[![](https://i.ytimg.com/vi/FNRje14J34g/0.jpg)](https://www.youtube.com/watch?v=FNRje14J34g)

### Beacons Active Mode - Click on the image to launch the video.

[![](https://i.ytimg.com/vi/Vpn9oOLJUsw/0.jpg)](https://www.youtube.com/watch?v=Vpn9oOLJUsw)

### Beacons In Daylight Mode - Click on the image to launch the video.

[![](https://i.ytimg.com/vi/ngtQo7elz8o/0.jpg)](https://www.youtube.com/watch?v=ngtQo7elz8o)


## Component list

Map and frame aside the following components will be required.

| Component | Quantity | Potential Source
| -------- | ------- | --------------|
| Capacitor 100nf (104) | 24 |
| Capacitor 1nf | 1 |
| ESP32 WROOM-32 30 Pin | 1 | https://www.ebay.co.uk/itm/235648899105 See the image below. I have been sent some that simply say "ESP-32" on the top and they have failed. If you buy one from here and it does not say "ESP-MODULE-32" like the picture below I would return it. You should be able to see this before opening the protective wrapper.
| Misc' Hardware - Nuts & Bolts, PCB standoffs etc. These requirements will depend on your build. | * | 
| LED Holders 8mm | 18 | https://www.ebay.co.uk/itm/196322906972?var=496149156991
| PCB - Beacon LED Board | 18 |
| PCB - Frequencies Board | 1 |
| PCB - Processor Board | 1 |
| PCB Mount Terminal Block 3-Way | 2 |
| PCB Mount Terminal Block 2-Way | 2 | 
| Resistors 75 Ohm 1/4 Watt | 24 |
| Switch 6x6x5mm Momentary PCB Tactile | 2 |
| Wiring Loom | three different coloured wires, 3m of each | 
| WS2812D 8mm LEDs | 24 | https://www.ebay.co.uk/itm/134876658482

Not that the ESP32 WROON-32 30 Pin should look like:

<img src="Pictures/AsExpected.jpg?sanitize=true&raw=true" />

## Wiring the LEDs

The LEDS on the main board must be wired in the following order:

ZL6B, VK6RBP, JA2IGY, VR2B, 4S7B, RR90
4X6TU, 5Z4B, ZS6DN, OH2B, CS3B, VE8AT
W6WX, KH6RS, 4U1UN, YV5B, OA4B, and LU4AA

For more information about the 8mm LEDs see: https://www.electrokit.com/upload/product/41020/41020706/41020706.pdf

## Installing the software

The source code for BeaconBox is not available. The software is distributed as a binary file
that can be downloaded directly to the ESP32 using esptool.exe that can be downloaded 
from https://github.com/espressif/esptool/releases. Download and install esptool.exe.

What follows is very specific to Microsoft Windows but similar tools and processes will apply
to MacOS etc.

Next connect the ESP32 to your computer using a USB cable. The device should appear as a COM port. 
You can use the Device Manager to identify thee new port - look for something called "USB-SERIAL CH340"
or similar. To be sure you have the correct COM port you could unplug it and see it dissapear and 
then plug it back in and see it appear. 

Next open a command window and type the command:

esptool.exe --chip esp32 --port COM4 --baud 921600 write_flash 0x0 BeaconBox_V0_7.bin

You may need to provide the complete path to the esptool.exe and you will probably need
to change COM4 to the COM port identified above.

## Issues

Please do not hesitate to contact me at martin@the-wallers.net is you have any problems.