# One-radar USB bench test

Unplug USB while wiring. Connect by the radar's printed signal labels, not assumed wire colours or connector left-to-right order.

| LD2450 | XIAO ESP32-C3 |
| --- | --- |
| 5V | 5V |
| GND | GND |
| TX | D7 / GPIO20 (RX) |
| RX | D6 / GPIO21 (TX) |

Connect the XIAO to the PC with a USB data cable. Do not connect the battery/power-bank supply for this bench test. The radar requires 5 V power and a supply capable of more than 200 mA for the radar itself; its UART uses 3.3 V logic, compatible with the XIAO. Never feed 5 V into an ESP GPIO or power the radar from 3V3.

Open `LD2450_USB_Test.ino` in Arduino IDE. Install **esp32 by Espressif Systems** in Boards Manager if absent. Select **XIAO_ESP32C3**, set **USB CDC On Boot = Enabled**, select its COM port and upload. Open Serial Monitor at **115200**. The independent radar UART runs at **256000, 8N1**. No extra library is required.

Walk in front of the radar with its antenna face pointing into the room. Every half-second the monitor shows the latest X/Y coordinates (mm), speed (cm/s) and distance-gate resolution (mm) for up to three target slots. The resolution field is not an accuracy claim. Target slot numbers are not persistent person identities. This passive test sends no configuration commands and assumes the radar's default baud rate.

`Connected; no targets` means valid radar frames are arriving. `NO FRESH RADAR DATA` means the UART link is not providing valid recent frames; check power, common ground, crossed TX/RX, selected pins and any previously changed radar baud rate. If the monitor is entirely blank, check USB CDC, the COM port and the USB data cable, then press RESET. If upload fails, hold BOOT while connecting USB, release it, select the new port and retry.

Protocol: 30-byte frames with `AA FF 03 00` header and `55 CC` trailer. Three 8-byte target records; X, Y and speed use little-endian magnitude with bit 15 indicating positive. An all-zero target record is empty. The protocol has no checksum; the sketch checks framing and resynchronizes after corruption, but cannot detect every payload error. Host timestamps are reception times, not measured sensor latency.

Sources: [Hi-Link manual, pages 9–13](https://www.tinytronics.nl/product_files/006000_HLK-LD2450-Instruction-Manual.pdf), [Seeed pinout](https://wiki.seeedstudio.com/XIAO_ESP32C3_Getting_Started/).

Status: compiled and uploaded to the connected XIAO ESP32-C3 on COM4 on 2026-09-16 using Arduino CLI and Espressif core 3.3.11, with USB CDC enabled. Flash verification passed. A USB serial capture confirmed live LD2450 target frames at approximately 11 frames/s, with changing X/Y positions and speed. This verifies basic communication and decoding, not calibrated position accuracy or latency. The serial port was closed after capture; the board continues running the test.

Evidence: `../../logs/ld2450_compile.log`, `../../logs/ld2450_upload.log`, and `../../logs/ld2450_serial_test.log`. Compiled size: 285,908 bytes flash and 13,472 bytes static RAM.
