# marax_timer

***This repository is a rewrite of [espresso_timer](https://github.com/alexrus/espresso_timer) that has better documentation and is easier to use.***

This project was designed to be used with a Lelit MaraX espresso machine (like the Lelit Mara PL62X), but it works (only the timer functionality) with any espresso machine that has a vibration pump.

## Hardware

The following hardware is needed:

- NodeMCU (ESP8266)
    - Tested with a NodeMCU V2 CP2102 board
- 0.96" OLED display
    - Tested with a 4-pin version
- Reed sensor
    - Normally closed and normally opened works
- Wires
- 3D printed case ([https://www.thingiverse.com/thing:2937731](https://www.thingiverse.com/thing:2937731))

### Hardware Connections

Connect the **OLED display** like this:
![](https://circuits4you.com/wp-content/uploads/2019/01/NodeMCU_ESP8266_OLED_Display.png)
Remember that different display versions have different orders of VCC, GND, and data pins.

The **reed switch** goes to ground and D7.

The reed switch will be glued onto the machine's vibration pump like this:
![](resources/pump.jpg)

If you have a Lelit MaraX machine, open the underside of the machine and you will see a 6-pin connector.


Connect pin 3 of the machine (RX) to pin D6 (TX) of NodeMCU

Connect pin 4 of the machine (TX) to pin D5 (RX) of NodeMCU

In case this does not work, change the pins connection (pin 3 to D5 & pin 4 to D6)


## Software

In order to make NodeMCU work with Arduino IDE, follow these instructions: [https://www.instructables.com/id/Quick-Start-to-Nodemcu-ESP8266-on-Arduino-IDE/](https://www.instructables.com/id/Quick-Start-to-Nodemcu-ESP8266-on-Arduino-IDE/)

### Operation

When the pump will start, the timer on the screen will also start counting.

When the pump is stopped, the time will remain printed on the screen (if more than 15 seconds have passed) until the next time the pump is activated.

The time is not recorded unless 15 seconds have passed because the MaraX machine starts the pump by itself from time to time to fill the boiler with water.

The display should go to sleep after 1 hour and will wake up when the pump is started.

If the timer immediately starts, even when the machine is off, a wrong reed sensor is used. To solve it, replace the reed sensor with another type (normally opened / normally closed) or change the `reedOpenSensor` to `true` in the `timer.ino` file.

### UI

![](resources/ui.jpg)


### Dependencies

This project uses the following dependencies:
* [https://github.com/adafruit/Adafruit_SSD1306](https://github.com/adafruit/Adafruit_SSD1306)
* [https://github.com/adafruit/Adafruit_BusIO](https://github.com/adafruit/Adafruit_BusIO)
* [https://github.com/adafruit/Adafruit-GFX-Library](https://github.com/adafruit/Adafruit-GFX-Library)
* [https://github.com/JChristensen/Timer](https://github.com/JChristensen/Timer)


### More information

More information about the project and progress can be found [here](https://www.home-barista.com/espresso-machines/lelit-marax-t61215-350.html#p723763). and [Here] (https://www.reddit.com/r/espresso/comments/jgtw4i/installed_a_diy_shot_timer_on_my_lelit_marax/).

# Lelit MaraX OLED Shot Timer (2026 Upgrade)

Forked from [alexrus/marax_timer](https://github.com/alexrus/marax_timer) with added Wi-Fi telemetry, mobile web dashboard, ntfy push alerts, wireless OTA updates, and an OLED burn-in screensaver.

## What's New
- **Push notifications (ntfy):** Get an alert on your phone when the machine reaches your desired target temperature. Customize the target temp, title, and message directly from the web dashboard.
- **Built-in web dashboard (`http://marax.local`):** View live brew/HX temp, steam temp, heating element status, and shot timer from any browser on your local network.
- **OLED burn-in screensaver:** Switches to a bouncing pixel-art espresso cup with animated steam 10 seconds after the machine is turned off, or after 1 hour of idle time while powered on (still showing live HX temp). Lifting the brew lever wakes the main display back up.
- **Wireless OTA firmware updates (`http://marax.local/update`):** Flash future `.bin` firmware updates directly over Wi-Fi without removing the ESP8266 from the machine.

## Required Arduino Libraries
- `Adafruit_GFX`
- `Adafruit_SSD1306`
- `Timer` (Simon Monk)
- Built-in ESP8266 Core libraries (`ESP8266WiFi`, `ESP8266WebServer`, `ESP8266mDNS`, `ESP8266HTTPUpdateServer`, `ESP8266HTTPClient`, `EEPROM`, `SoftwareSerial`, `Wire`)

## Quick Configuration (After Flashing)
1. Enter your `ssid` and `password` at the top of `marax_timer.ino` before your initial USB upload.
2. On boot, the OLED displays its local IP address and `http://marax.local`.
3. Open `http://marax.local` in your phone's browser to view live stats and configure your **ntfy** topic, target temp (`°C`), and alert message.
4. For future updates, export a binary in Arduino IDE (**Sketch → Export Compiled Binary**) and upload it at `http://marax.local/update`.

---
