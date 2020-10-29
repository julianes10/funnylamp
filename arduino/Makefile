BOARD_TAG    = nano
MCU = atmega328p

ARDUINO_PORT = /dev/ttyUSB0
ARDUINO_LIBS = SoftwareSerial FastLed stringQueue lsem SimpleTimer EEPROM Adafruit_Sensor Low-Power DHT-sensor-library DHTsensor HCSR04sensor NonBlockingRTTTL/src Arduino-MedianFilter/src  RtttlTrackerList U8g2_Arduino SPI Wire RTClib Adafruit-GFX-Library Adafruit_SSD1306 Adafruit_BusIO arduino-DHT

USER_LIB_PATH = $(realpath ../SCS/arduino)

AVRDUDE_ARD_BAUDRATE = 115200

ARDUINO_DIR=/home/jes/arduino-1.8.13
ARDMK_DIR=/usr/share/arduino



#Cheatsheet 
#CPPFLAGS -c pre procesor-, should be on for any compilation C or C++. Not for linking
#CXXFLAGS compiling and linking c++
#CFLAGS   compiling and linking c
#Some flags: -Os optimized in size, setup by default 
#Some flags: -O3 optimized in speed (others 1,2 available also)



############## CONDITIONAL COMPILATION
# As the code and ram is at limit of nano, i must play with ugly ifdefs...
# LAMP_ENABLE_LSLIB
# LAMP_ENABLE_SERIAL_INPUT
# LAMP_ENABLE_EXTRAMUSIC
# LAMP_ENABLE_DISPLAY
# LAMP_ENABLE_RTTTL
# LAMP_ENABLE_RTC
# LAMP_ENABLE_HCSR04
# LAMP_ENABLE_DHT

# LAMP_DEBUG_LOOP
# LAMP_DEBUG_STATES
# LAMP_DEBUG_HCSR04_MEDIAN
# LAMP_DEBUG_TEMPCOLOR
# LAMP_DEBUG_RTC
# LAMP_DEBUG_DHT
# LAMP_DEBUG_HCSR04
# LAMP_DEBUG_LS
# LAMP_DEBUG_STILLALIVE

# HCSR04_ENABLE_MEDIANFILTER --->dangerous even with 77% ram may cause problems!
# SIMPLETIMER_ENABLE_10/5/...1 
# DHTSENSOR_ENABLE_LOWCOST22
# NONBLOCKINGRTTTL_ENABLE_MINBUFFER
# LSEM_ENABLE_LIGHT
# LSEM_DEBUG
# STRINGQUEUE_ENABLE_3
############## 
CPPFLAGS+= -DLAMP_ENABLE_DISPLAY -DLAMP_ENABLE_HCSR04 -DLAMP_ENABLE_RTTTL -DLAMP_ENABLE_RTC -DLAMP_ENABLE_DHT -DLAMP_ENABLE_LSLIB
CPPFLAGS+= -DLAMP_DEBUG_LOOP  #-DLAMP_DEBUG_STILLALIVE #-DLAMP_DEBUG_HCSR04 #
CPPFLAGS+= -DSIMPLETIMER_ENABLE_2 -DNONBLOCKINGRTTTL_ENABLE_MINBUFFER -DSTRINGQUEUE_ENABLE_3 -DLSEM_ENABLE_LIGHT  #-DDHTSENSOR_ENABLE_LOWCOST22


include $(ARDMK_DIR)/Arduino.mk



