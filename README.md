# funnylamp

Clock Lamp with the following functions:

- Display time / temperature (2 sources) / humidity and if alarm is enabled or not.
- Clock alarm that consists of up to 120 seconds of rtttl songs. It can be stopped approaching ultrasonic sensor also.
- Alarm: to enable/disable alarm just hold your hand 5 seconds near to ultrasonic. Display confirm you the change. 
- In idle state it show different colors depending on temperture, after 10 seconds it turn a bit lower light.  Led show has much more sense if you can use a case with a litophane (tutorial at https://www.youtube.com/watch?v=y63sVpeViXo&t=370s, Tool: http://3dp.rocks/lithophane/ ) 
- Sing miscelaneus (rtttl) songs when ultrasonic sensor is excited in specific distance range
- To change the song approach ultronic sensor with medium disntace
- To stop the song approach ultronic sensor with short disntace.
- To change the speed of the song approach ultronic sensor an move from medium-short to medium-large distance or viceversa.


## Code disclaimers
A lot of conditional compilation has been needed to debug and tune properly final project. This makes a bit ugly the code but needed.... Also massive usage of nasty global vars is also convinient to avoid bigger stack usage.

## Caution notes to remember
CAUTION: never try to upload an sketch beyond 31.5K, it will damage nano bootloader.
CAUTION: ram above 80% makes much easier an stackoverflow.

## Hardwawe
- Arduino nano
- Temperature and humidity sensor DHT22
- HCSR04 ultrasonic sensor
- Active buzzer
- RGB led strip WS2812
- basic white leds strip for adding more white light
- Real time clock (include temperature sensor and battery) RTC DS3231
- Display SSD1306 OLED I2C 128X32

Home made case: 3D project at: https://www.tinkercad.com/things/hfVAcqovB4N-funnylamp

## Known Limitations
- Time setup. no way, suggest compile and load SCS/basicExamples/rtcSetTimeBuild
- configuracion of alarm time: suggest to compile


## TODO / BACKLOG DISCARDED IDEAS
Features discarded by lack of flash / ram / time
- Sing (rtttl) 2digit number (temperature and humidity) when keep some seconds at specific range distance
- Better light effects e.g using SCS lsem library with protocol...
- EEPROM usage for store alarm enable/disable, by default is disabled at boot time now.
- BT for funny interface with mobiles. BT usage:
 - report temp tracking
 - trigger play dummy songs in buzzer
 - trigger play different coloring modes
- motor to turn lamp case
- basic serial interface for configuring e.g datetime and alarms.
Config things:
- movement on/off and timings
- sound melody at power on (of off)
- light colors on/off
- push server id
- push timmings
- power mode saving, check if possible use batteries...

