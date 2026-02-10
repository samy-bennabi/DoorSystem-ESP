# NFC Door lock
Microcontroller portion of a whole system to unlock doors electronically with NFC cards.

# Installation
## Prerequisites
- ESP32.
- MFC522 RFID card reader.
- Any electronically actuated lock.
- 12v relay.
- Backend Server.

## Setup
### Hardware
Connect all of the components to the ESP32 according to this diagram:
##### gonna insert diagram here when I make it.

### Firmware
If you have another ESP model, or you have chosen to use different pins than the ones shown in the diagram, be sure to change the appropriate variables.
Have a wifi network ready for the ESP to connect to, and give it the credentials (directly in the code or by setting up a connection when the ESP enters AP mode automatically upon failing to connect to wifi after a few tries).
Flash the code onto your ESP (via pio cli, a pio extention in vscode, or the arduino IDE).

## Usage
With a woking backend server, scanning an authorised card will send the signal to unlock the door.


will make the readme better and more useful when I feel like it.