# ESP32-SD-CAN-Examples
ESP32-SD-CAN Arduino examples

# Setting Up Arduino IDE for the ESP32-C6-MINI-1

A step-by-step guide to programming an ESP32-C6-MINI-1 module (or a dev board built around it, such as the ESP32-C6-DevKitC-1) using Arduino IDE 2.x.

## About the Chip

The ESP32-C6-MINI-1 is a module built around the ESP32-C6 SoC:

- 32-bit RISC-V CPU (up to 160 MHz) plus a low-power RISC-V core
- Wi-Fi 6 (802.11ax), Bluetooth 5 (LE), and IEEE 802.15.4 (Zigbee / Thread / Matter)
- 4 MB flash on the standard MINI-1 variant
- Native USB Serial/JTAG (no external USB-UART chip required on the chip itself)

> **Important:** ESP32-C6 support requires **Arduino-ESP32 core 3.0.0 or newer**. Older 2.x cores do not include the C6.

---

## Prerequisites

- A computer running Windows, macOS, or Linux
- An ESP32-C6-MINI-1 dev board (or a module on your own PCB with a way to program it)
- A **data-capable** USB cable (many cheap cables are charge-only)

---

## Step 1: Install Arduino IDE

1. Download Arduino IDE 2.x from <https://www.arduino.cc/en/software>.
2. Install and launch it.

---

## Step 2: Add the ESP32 Board Package URL

1. Open **File → Preferences** (macOS: **Arduino IDE → Settings**).
2. In **Additional boards manager URLs**, add:

   ```
   https://espressif.github.io/arduino-esp32/package_esp32_index.json
   ```

3. If a URL is already present, click the icon next to the field and put each URL on its own line.
4. Click **OK**.

---

## Step 3: Install the ESP32 Core

1. Open the **Boards Manager** (left sidebar, or **Tools → Board → Boards Manager…**).
2. Search for **esp32**.
3. Find **esp32 by Espressif Systems** and click **Install**.
4. Confirm the installed version is **3.0.0 or higher**.

The download is large (several hundred MB including toolchains), so it may take a few minutes.

---

## Step 4: Connect the Board

1. Plug the board into your computer with a USB C data cable.

### Driver Notes

| OS | Notes |
|----|-------|
| Windows 10/11 | Usually automatic. If the port doesn't appear, install the [Silicon Labs CP210x driver](https://www.silabs.com/developers/usb-to-uart-bridge-vcp-drivers) (or the driver for whichever bridge chip your board uses). |
| macOS | Usually works without drivers. |
| Linux | Add your user to the serial group, then log out and back in: `sudo usermod -a -G dialout $USER` (on some distros the group is `uucp`). |

---

## Step 5: Select the Board and Port

1. Go to **Tools → Board → esp32** and choose **ESP32C6 Dev Module**.
2. Go to **Tools → Port** and select the serial port for your board:
   - Windows: `COMx`
   - macOS: `/dev/cu.usbserial-*` or `/dev/cu.usbmodem*`
   - Linux: `/dev/ttyUSB0` or `/dev/ttyACM0`

---

## Step 6: Configure Tools Menu Settings

Recommended settings for the ESP32-C6-MINI-1 (4 MB flash):

| Setting | Recommended Value |
|---------|-------------------|
| Board | ESP32C6 Dev Module |
| USB CDC On Boot | **Disabled** if using the UART port; **Enabled** if using the native USB port |
| CPU Frequency | 160 MHz |
| Core Debug Level | None (raise for troubleshooting) |
| Erase All Flash Before Sketch Upload | Disabled |
| Flash Frequency | 80 MHz |
| Flash Mode | QIO |
| Flash Size | **4MB (32Mb)** |
| JTAG Adapter | Disabled |
| Partition Scheme | Default 4MB with spiffs |
| Upload Speed | 921600 (drop to 460800 or 115200 if uploads fail) |
| Zigbee Mode | Disabled (unless you're using Zigbee) |

> **Tip:** If you want `Serial.print()` output over the **native USB** port, you must set **USB CDC On Boot: Enabled**. Otherwise `Serial` maps to UART0 (GPIO16 TX / GPIO17 RX), which is what the UART port on the dev board uses.

---

## Step 7: Upload a Test Sketch (Blink)

The DevKitC-1 has an addressable RGB LED on **GPIO8**:

```cpp
#include <FastLED.h>

#define NUM_LEDS 1
#define LED_PIN 8

CRGB leds[NUM_LEDS];

void setup() {
  FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, NUM_LEDS);
}

void loop() {
  leds[0] = CRGB::Green;
  FastLED.show();
  delay(500);

  leds[0] = CRGB::Black;  // off
  FastLED.show();
  delay(500);
}
```

Steps:

1. Paste the code into a new sketch.
2. Click **Upload** (right arrow).
3. Open the **Serial Monitor** (**Tools → Serial Monitor**) and set the baud rate to **115200**.

> If your board or custom PCB has a plain LED, replace the `neopixelWrite` calls with `pinMode(pin, OUTPUT)` and `digitalWrite(pin, HIGH/LOW)` on the correct GPIO.

---

## Troubleshooting

### Port doesn't appear
- Try a different USB cable (must carry data).
- Try the other USB-C port on the dev board.
- Install the appropriate USB-UART driver (see Step 4).
- On Linux, check group permissions (`dialout`/`uucp`).

### "Failed to connect to ESP32-C6: Wrong boot mode detected" / upload times out
Put the chip into download mode manually:

1. Hold the **BOOT** button .
2. Tap **RESET** (EN) while still holding BOOT.
3. Release BOOT.
4. Start the upload again.

After upload, press **RESET** to run the sketch.

### Nothing in Serial Monitor
- Check the baud rate matches `Serial.begin()`.
- If using the native USB port, set **USB CDC On Boot: Enabled** and re-upload.
- If using the UART port, set it to **Disabled**.
- After upload over native USB, the port may re-enumerate. Reselect the port under **Tools → Port**.

### Garbage characters in Serial Monitor
- Baud rate mismatch. Match the monitor to your `Serial.begin()` value.

### Boot loop or crash on startup
- Set **Core Debug Level: Verbose** and read the log.
- Confirm **Flash Size** is 4MB and the partition scheme fits it.
- Try **Erase All Flash Before Sketch Upload: Enabled** for one upload, then set it back.

### Board package won't install
- Check the Additional Boards Manager URL for typos.
- Check your network/proxy/firewall settings.
- Restart Arduino IDE and retry.

---

## Useful Links

- Arduino-ESP32 documentation: <https://docs.espressif.com/projects/arduino-esp32/en/latest/>
- ESP32-C6 datasheet and resources: <https://www.espressif.com/en/products/socs/esp32-c6>
- Arduino-ESP32 GitHub: <https://github.com/espressif/arduino-esp32>