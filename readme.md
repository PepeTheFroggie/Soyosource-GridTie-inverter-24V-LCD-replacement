This is a work in progress. Onyl for the 24V 1000W soyo inverter with LCD. The lcd-less type use other messages. Absolutely no liability. Your fuckups are your problem.

Serial console tells you where to connect. And use your own wifi credentials, not mine.

Connect soyosource rx and tx via a 10kohm resistor to the esp8266 Tx = GPIO15, Rx = GPIO13.

This pins:
![wiring.png](wiring.png "This way")

That cable:

![images.jpg](images.jpg "This cable")

I do not remember which cable pins are what signal, find it yourself. 

Main screen:

![Clipboard02.jpg](Clipboard02.jpg "Hi")

Settings screen, press "read values" to see:

![Clipboard03.jpg](Clipboard03.jpg "Hi")

Graph screen, red=volt, green=amps, magenta=power:

![Clipboard01.jpg](Clipboard01.jpg "Hie")

Good luck. If you find improvements please notify.

Do not use "Bat power" setting for dynamic adjustments, this will fuck up your eeprom. There might be a special message for dynamic power setting but i have not tested that. 
