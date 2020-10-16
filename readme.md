# cub2bed_client
## What is cub2bed_client? 
Simple two-way message appliance

### Purpose
Client message appliance alerts server appliance of message and receives status updates from server

### Contributors

### Software Dependencies not in Arduino Library Manager

### BOM
- 1x: Adafruit Feather Huzzah 8266 https://www.adafruit.com/product/2471
- simple button
- 1 protoboard
- wire

### Pinouts

### Information Sources

### Issues
- 071620[P2]; need to step up 3.3v to 5v for the button LED
- 101220[P3]; AdafruitIO feeds not setup properly
- 101320[P1]; cub2bedSub pulls last-1 message, not last message, causing logic issues
- 101520[P1]; If cub2bedPub fails, downstream is not handled properly
- 101520[P2]; statusLightPub On should be handled by c2b server, not client, ensuring light only goes on when the server has recognized cub2bed message from client

### Questions

### Learnings
- 071620 - Breadboard makes box fit not; 60mm might be tight either way. Might want to take z-axis to 65mm
- 101220 - Seem to be having trouble with button input on multi-purposed ESP8266 pins. No issues on SCL/SDA

### Feature Requests
- 101220[P2]; MQTT QoS 1
 
### Revisions
- 101220
	- new code branched from cub2bed_client
	- messaging via 900mhz radio removed
	- messaging via MQTT topic added
- 101520
	- MQTT code improvements