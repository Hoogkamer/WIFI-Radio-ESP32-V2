# WIFI Radio

ESPF32 Wifi radio

## Description

A fully functional internet radio with LCD. Keeping it as simple as possible.  
If you want a version with more functionalities (and a lot of more code, that is not my project): [ESP32-MiniWebRadio V2](https://github.com/schreibfaul1/ESP32-MiniWebRadio)

Features:

- Use a remote to operate and/or rotary buttons
- Radio stations configurable with website
- Each station can have a category. You can switch +/- a radio station within the category
- You can switch +/- between categories
- You can save 1 station which is then selected when you switch on the device
- You can specify images for the categories and the radio stations (via the optional SD card)

### Radio1: Drop-in for an IPOD with remote

![RadioRemote](/readme_info/images/radioandremote.jpg)

### Radio2: Portable radio with rotary buttons

![RadioRotaries](/readme_info/images/RadioRotaries.jpg)

## Hardware and pinouts

All components are less than 25 EURO. The donor radio was 10 EURO second hand. So the total cost of this fun project was 35 EURO. And a lot of hours :)

### ESP32:

Diymore ESP32 Ontwikkelingsplaat NodeMCU Module 2,4 GHz Dual Core WLAN WiFi Bluetooth CP2102 Chip ESP32 WROOM

[ESP32 Amazon](https://www.amazon.nl/dp/B0BZRJ1H16?psc=1&ref=ppx_yo2ov_dt_b_product_details)

### ESP32 with antenna

Or you can choose with an external antenna to improve the WIFI signal receiving

https://nl.aliexpress.com/item/1005003106292591.html?spm=a2g0o.order_list.order_list_main.29.7b2a79d2YE7TfF&gatewayAdapt=glo2nld

https://nl.aliexpress.com/item/1005002410521023.html?spm=a2g0o.order_list.order_list_main.53.2b0279d2JmJ4ml&gatewayAdapt=glo2nld
(choose the one with external antenna connection)

### LCD:

Ili9341 240\*320 2.8 inch touch

[LCD Aliexpress](https://nl.aliexpress.com/item/33015586094.html?spm=a2g0o.order_list.order_list_main.10.2b0c79d2hiKNqB&gatewayAdapt=glo2nld)

### DAC:

[I2S PCM5102A Dac Decoder](https://nl.aliexpress.com/item/1005002898278583.html?spm=a2g0o.order_list.order_list_main.17.21ef79d2UT6dOU&gatewayAdapt=glo2nld)

### IR Receiver (optional):

10Pcs Universele Ir Infrarood Ontvanger TL1838 VS1838B 1838 38Khz

[IRR Aliexpress](https://nl.aliexpress.com/item/1005003194864725.html?spm=a2g0o.order_list.order_list_main.23.2b0c79d2hiKNqB&gatewayAdapt=glo2nld)

### SD Card Reader (optional):

AZDelivery 3 x SPI Reader Micro Memory SD TF Module Card

[SD Amazon](https://www.amazon.nl/dp/B077MCQS9P?psc=1&ref=ppx_yo2ov_dt_b_product_details)

### Two Rotary Encoder (optional):

Used for volume and tuning the radio station. Instead you can use the remote/IR receiver option

KY-040 360 Graden Rotary Encoder Module
[Rotry Encoder Aliexpress](https://nl.aliexpress.com/item/4000332297021.html?spm=a2g0o.order_list.order_list_main.35.235979d2rPFAs1&gatewayAdapt=glo2nld_)

### Amplifier (for portable radio)

All original electronics of the portable radio is removed, so it does not have an amplifier anymore. You should connect an amplifier to the DAC (and speaker)

[5128 Amplifier board](https://nl.aliexpress.com/item/1005004428568482.html?spm=a2g0o.order_list.order_list_main.23.21ef79d2UT6dOU&gatewayAdapt=glo2nld)

## Used pinouts

| ESP32 | LCD         | PCM             | IR        | SD      | wire colors | Rotary1 | Rotary2 |
| ----- | ----------- | --------------- | --------- | ------- | ----------- | ------- | ------- |
| v5    | 1- VCC      | 6- VIN          | 3- VCC    | 5- VCC  | red         |         |         |
| GND   | 2- GND      | 5- GND + 1- SCK | 2- GND    | 6- GND  | black       |         |         |
| 22    | 3- CS       |                 |           |         | blue        |         |         |
| EN    | 4- RESET    |                 |           |         | yellow      |         |         |
| 21    | 5- DC       |                 |           |         | green       |         |         |
| 23    | 6- SDI/MOSI |                 |           | 3- MOSI | brown       |         |         |
| 18    | 7-SCK       |                 |           | 2- SCK  | grey        |         |         |
| 32    | 8-LED       |                 |           |         | purple      |         |         |
| 19    | 9-SDO/MISO  |                 |           | 4- MISO | white       |         |         |
| 5     |             |                 |           | 1- CS   | orange      |         |         |
| 26    |             | 4- LCK          |           |         |             |         |         |
| 25    |             | 3- DIN          |           |         |             |         |         |
| 27    |             | 2- BCK          |           |         |             |         |         |
| 15    |             |                 | 1- SIGNAL |         |             |         |         |
| 13    |             |                 |           |         |             | 1-CLK A |         |
| 17    |             |                 |           |         |             | 2-DT B  |         |
| 35    |             |                 |           |         |             | 3-SW    |         |
| v3    |             |                 |           |         |             | 4-+     | 4-+     |
| GND   |             |                 |           |         |             | 5-GND   | 5-GND   |
| 14    |             |                 |           |         |             |         | 1-CLK A |
| 34    |             |                 |           |         |             |         | 2-DT B  |
| 33    |             |                 |           |         |             |         | 3-SW    |

dont use 0,2,12,15

(wire colors you can use as desired)

![ESP32-1](/readme_info/images/ESP32-1.jpg)
![ESP32-2](/readme_info/images/ESP32-2.jpg)
![PCM](/readme_info/images/PCM.jpg)

- open filezilla
- put in ip address (select "input level" on remote to see it)

## Install PlatformIO in VS Code

This you need for compiling the code and transferring it to the ESP32.

[install platformio in vscode](https://randomnerdtutorials.com/vs-code-platformio-ide-esp32-esp8266-arduino/)

## clone the github repository

In a terminal window:

> git clone https://github.com/Hoogkamer/wifiradio.git

## Changing the code

### changing the header file

You can specify the options for the radio in the header file.
For both radios an example header file is provided (and you should include one of them in the main1.h file)

- ipodradio.h: this is the first radio example with remote
- portableradio.h: this is the second radio example with rotary buttons

### Settings for the remote

If you want to use a different remote you can replace the remote codes.
The platformio will display the remote control code of the button you are pressing in the Serial Monitor.
Provide it in the header file (for example ipodradio.h)

### Uploading the Filesystem image

This contains the website.
In VSCODE:
PlatformIO > ESP32 > Platform > Build Filesystem Image, and then Upload Filesystem Image

This will uploade everything in the "data" folder
[More] (https://randomnerdtutorials.com/esp32-vs-code-platformio-spiffs/)

### Preparing the SD Card

The SD card is optional, you do not have to use this.

See the folder "SD", copy this to the SD card. Root folder copied should be "wifiradio".

#### Using images for station categories and station names

in the \wifiradio\img\category folder, but the picture to display for the category with exactly the category name and ending with ".jpg"
Picture for category should be 240x80.

in the \wifiradio\img\radio folder, put the picture to display for the radio with exactly the radio name and edingn with ".jpg"
Resolution of each image should be 240 pixels wide and max 240 pixels high. Note that there are only 2 example images for the radio station, due to potential copyright issues.
You can copy your own images and scale them to 240px.

## Operating the Internet radio

### Connecting to a new WIFI access point

When the Radio cannot connect to a wifi access point, it will startup an access point "WIFI_RADIO"
Connect with your Phone/Computer to that access point and navigate to 192.168.1.4

[More](https://github.com/tzapu/WiFiManager#using)

### Showing the IP address

Shows the screen with IP address how you can access the webpage.

#### Using Remote

You can configure the button yourself. In this case it is mapped to the Ipod MENU button.

#### Using Rotary button

Or Volume Rotary button press.

### Editing radiostations

Navigate on your phone/computer to the IP address mentioned above. You must be connected to the same WIFI access point.
Then go to edit stations and wait until you see the categories dropdown populated. If not after a while, refresh the page.
Edit the stations and press "Save" to upload it to the WIFI Radio. Use HTTP, not HTTPS links to the streams!

### Changing categories and radio stations

#### Using Remote

This depends on the buttons you want to use of the remote.
In this example the stations are selectable by the left-right keys. And the categories by the up-down keys.
It only selects it after 3 seconds so you can quickly navigate through the stations (otherwise it halts untill the streaming is workding)
You can also activate with the OK button, see next section.

#### Using Rotary

Click the Station Rotary button to change the Category. Turn the Station Rotary button to change the station.

### Stopping the radio stream

#### Using Remote

Press the STOP button on your remote. Pressing OK starts the stream again.

#### Using Rotary

Press and hold Volume Rotary

### Activating the station

#### Using Remote

After selecting the station, It will automatically connect after 3 seconds. Or press the OK button on your remote to select it.

#### Using Rotary

It will automatically connect after 3 seconds

### Saving a radio station

#### Using Remote

You can configure the button yourself. In this case it is mapped to the Auto Preset button.

#### Using Rotary

Press and hold the Station Rotary Button.

## Donor radio 1 (IPOD)

Panasonic SC-HC3
It sounds great. It has a radio, CD player and a room for an IPOD behind a sliding door.
Fits all the components needed. You can:

- Remove all internals and use the speakers and housing only. You need to add an amplifyer then. And you can only use it as an internet radio
- Keep everything working and build it in the docking station compartment and use an external USB charger.

I opted for the second option as I also want to be able to use the CD player still.

[SC-HC3](https://support-nl.panasonic.eu/app/products/detail/p/20174)

### Creating the "IPOD"

You first need to find a case where you can contain the components in. I used a old VHS tape case as the thickness is just right.

- cut it to size with a jigsaw/sharp (hot)knife
- cut out the hole for the LCD screen and a small hole for the IR led
- (hot) glue the display to the bottom and the card reader to one of the sides
- then fit in the audio board at the bottom and finally put the ESP32 on top

![IPOD](/readme_info/images/IPOD.jpg)
![IPOD](/readme_info/images/IPOFRONT.jpg)
![IPOD](/readme_info/images/npo.jpg)

### Feeding the wires through the radio

First remove the speaker grills, they are clipped on, so just pull them off.

Then move the sliding door into the CD open position by pressing CD OPEN. Then on the CD side, put your finger under the door and bend the middle of the door towards you (away from the radio) then you can slide it further to the left.

OR

Look at the underside of the door and see the indent where you can see a white gear. Move it to one side with a screw driver, until you free up the door so you can slide it to one side.

[Video](https://www.youtube.com/watch?v=7ARk9byE3X4)

Then remove the 5 screws and also the 8 small screws of the silver top and bottom edges.

![RadioOpen1](/readme_info/images/radioopen1.jpg)

You can also remove the ipod connector adapter, so you have room to remove the docking adaptor piece at the bottom, so freeing up some more space. It is probably not needed to do. If you remove the docking adaptor, notice the switch at the bottom right. You eighter need to short the connection from the inside, or glue the switch in the closed position. If you do not do this the radio thinks the docking station is open and it gives an error message so the sliding door does not work.

Then drill a hole large enough for the audio cable and the usb cable from the front side, and a hole from the backside.

![cables front](/readme_info/images/cablesfront.jpg)

![cables back](/readme_info/images/cablesback.jpg)

### Possible improvements

- Power the ESP32 via the IPOD docking connector wires. This can be risky as the available power is only 0.5A, and I am not sure how much the ESP with screen etc is drawing.
- Connect to the AUX port internally instead of using a cable running outside the case.

## Donor radio 2 (Portable)

A Grundig music boy

### Removing old stuff

Remove screws at the bottom. Also the 2 knobs and screws at the top and lift the whole top. Then buy a piece of plexiglass, and make it the fit the top properly, cut 3 holes (1 for antenna, 2 for the rotary knobs). Paint the backside of the plexiglass black or any other color, leaving the room free for the LCD screen.
Then solder everything etc.

![Rotaries open](/readme_info/images/RotariesOpen.jpg)
