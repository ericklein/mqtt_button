# cub2bed_client
## What is cub2bed_client? 
Simple two-way message appliance

### Purpose
First user alerts the second user to come2bed via status-light, and the second user responds with positive or negative response that the first user can see.

### Contributors

### Software Dependencies not in Arduino Library Manager

### BOM
- 1x: Adafruit Feather Huzzah 8266 https://www.adafruit.com/product/2471
- simple button
- 1 protoboard
- wire

### Pinouts
- button input on pin 5 (see learnings)
- button LED on pin 16
- GND to button input, LED

### Information Sources

### Issues
- [P2]071620: button too dim (resistor, change LED?)
- [P1]101320: cub2bedSub pulls last-1 message, not last message, causing logic issues?
- [P2]101520: If cub2bedPub fails, downstream is not handled properly
- [P2]101520: statusLightPub On should be handled by c2b server, not client, ensuring light only goes on when the server has recognized cub2bed message from client
- [P2]102420: Move local MQTT server to named entry instead of IP address so DNS can resolve it if entry changes
- [P2]110620: Client doesn't have a timeout to reset its state if no server message is received
- [P1]122320: Client appears to light the button every 10 seconds for less than 1 second

### Feature Requests
- [P3]101220: MQTT QoS 1

### Questions

### Learnings
- 071620 - Breadboard makes box fit not; 60mm might be tight either way. Might want to take z-axis to 65mm
- 101220 - Seem to be having trouble with button input on multi-purposed ESP8266 pins. No issues on SCL/SDA
 
### Revisions
- 101220
	- new code branched from cub2bed_client
	- messaging via 900mhz radio removed
	- messaging via MQTT topic added
- 101520
	- MQTT code improvements
- 101620
	- [I]101620 [P1]; Example ping code fires every loop() but docs say to ping infrequently within the keepalive window to avoid packet collision -> add timer loop to ping at MQTT_KEEP_ALIVE
- 103020
	- [I]101720 [P2]; blue LED on Huzzah board needs to be supressed (or piped!) -> moving button LED to pin 16 from 2 to separate from on-board LED. Added code to disable on-board LED.
	- [FR]051920 [P3]; Use the builtin LED as a crude indicator (packet send, receive, etc.) -> closed as duplicate to 102420 P2 issue
- 010221
	- [FR]100120 [P3]; validate Ethernet code -> code aligned with functioning status_light Ethernet code
	- [FR]111020 [P3]; combine MQTT publish code blocks for AdafruitIO MQTT and generic MQTT -> complete
	- [I]101220 [P3]; AdafruitIO feeds not setup properly, see Publish setup code -> closed because of MQTT code merge
	- [I]102420 [P1]; Need an error indicator for for non-DEBUG, while(1) errors, MQTT connection errors -> while(1) error handling improved and now blinks button LED in unique pattern to indicate fatal error
