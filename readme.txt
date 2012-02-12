Release 0.952 changelist
Fixed bug - owaddress appearing in BSC message when no SRAM/FRAM. Not possible as owaddress is stored in SRAM/FRAM only.
Tidy up.
Changed number of sensors from 4 to 6 for non SRAM/FRAM build (tested to 5).
-----------------------------------------------------------
Release 0.951 changelist
Added freeram to xAP heartbeat message (only if DEBUG_THIS is #defined)
-----------------------------------------------------------
Release 0.950 changelist
Conversion to Ethercard
-----------------------------------------------------------
README.TXT 0.947
First Github release

These files are for the xAP interface to the Dallas one wire temperature sensors.

A single compile time option allows for the use of an external FRAM or SRAM and it has been checked against both.
Another compile time option allows additional debug to be captured and output. Be aware than enabling this brings the code size up to within a few bytes of the max program memory.
xAP 1.2 and 1.3 are both supported via a complile #define. Most of the testing has taken place with xAP 1.3

Using a board other than a Nanode has not been tested.

Using static IP also has not been tested.

