# ESP8266-DC-Motor-Position-Control

Additions and comments for original project by Whitebank

https://create.arduino.cc/projecthub/whitebank/dc-motor-position-control-986c10<br>
<br>
Original author https://create.arduino.cc/projecthub/whitebank<br>
<br>
Commented 17-1-2021 Quercus62
<br>
Tested on NodeMCU V3 new version small footprint<br>
Arduino IDE 1.8.13<br>
ESP boards 2.7.4<br>
FS 4MB (FS:3MB OTA...)<br> 
<br>
Hardware attached:<br>
- H-Bridge Steppermotor Dual Dc Motor Driver Controller Board HG7881 2.5-12V (no PWM)<br>
- Small DC index motor from a scanner<br>
<br>
Problems with original sketch:<br>
- Compiled oké but crashed immediately<br>
- PWM removed -> not on HG7881<br>
- Added ICACHE_RAM_ATTR in declaration of the interupthandles (line 51)<br>
- Line 133 analogWrite(LED, rotation_value); removed, analogWrite ESP8266 in combination with interupts causes crash?<br>


  
  
 
