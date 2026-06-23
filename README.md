# NFC Door lock

Electronic door locks with keycards.
This portion is for the microcontroller that controls the card reader.
Check out also;

> [Backend API]()
> [Web console]()

# Installation
## Prerequisites
- ESP32 microcontroller.
- MFC522 RFID card reader.
- Electronically actuated lock (and a way to power it if your microcontroller can't output enough).

## Setup
### Hardware
Connect all of the components to the ESP32 according to this diagram:
##### gonna insert diagram here when I make it.

### Firmware
If you have another ESP model, or you have chosen to use different pins than the ones shown in the diagram, be sure to change the appropriate variables.
Have a wifi network ready for the ESP to connect to, and give it the credentials (directly in the code or by setting up a connection when the ESP enters AP mode automatically upon failing to connect to wifi after a few tries).
Flash the code onto your ESP (via pio cli, a pioarduino extention in vscode, or the arduino IDE).

## Usage
Scan an authorised card and the door opens!


will make the readme better and more useful when I feel like it.