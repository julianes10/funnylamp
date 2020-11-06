#include <stdio.h>
#ifdef LAMP_ENABLE_LSLIB
#include "lsem.h"
#endif
#include "LowPower.h"
#include "SimpleTimer.h"

#ifdef LAMP_ENABLE_DHT
#include <Adafruit_Sensor.h>
#include <DHT.h>
#endif

#ifdef LAMP_ENABLE_HCSR04
#include "HCSR04sensor.h"
#endif

#ifdef LAMP_ENABLE_RTTTL
#include "NonBlockingRtttl.h"
#include "RtttlTrackerList.h"
#endif

#include <Arduino.h>
#include <Wire.h>                        // Include Wire library (required for I2C devices)

#ifdef LAMP_ENABLE_DISPLAY
#include <U8g2lib.h>
#endif
#ifdef LAMP_ENABLE_RTC
#include "RTClib.h"
#endif




//------------------------------------------------
//--- DEBUG  FLAGS     ---------------------------
//------------------------------------------------
// Check makefile

//------------------------------------------------
//--- GLOBAL VARIABLES ---------------------------
//------------------------------------------------


// PINS 
#define PIN_LED_WHITE             5 

#ifdef LAMP_ENABLE_RTTTL
#define PIN_BUZZER                8
#endif


// Sensors
#ifdef LAMP_ENABLE_DHT
#define PIN_SENSOR_TEMP           6
DHT GLBsensorDHT(PIN_SENSOR_TEMP, DHT22);
#endif
float GLBsensorDHTTemp=0.0;
float GLBsensorDHTHum=0.0;


#ifdef LAMP_ENABLE_HCSR04
#define MEDIAN_FILTER_SIZE_SENSOR_HCSR04  3
#define INT_SENSOR_HCSR04         0
#define PIN_SENSOR_HCSR04_TRIGGER 3
#define PIN_SENSOR_HCSR04_ECHO    2  //INT NEEDED
HCSR04sensor GLBsensorHC(PIN_SENSOR_HCSR04_TRIGGER,PIN_SENSOR_HCSR04_ECHO,INT_SENSOR_HCSR04,MEDIAN_FILTER_SIZE_SENSOR_HCSR04);
#endif
uint16_t GLB_HCdistance=0;

#ifdef LAMP_ENABLE_DISPLAY

U8G2_SSD1306_128X32_UNIVISION_F_HW_I2C u8g2(U8G2_R0, /* clock=*/ SCL, /* data=*/ SDA, /* reset=*/ U8X8_PIN_NONE);  
#endif 


#ifdef LAMP_ENABLE_RTC
RTC_DS3231 rtc;
DateTime GLB_RTCnow;
#else
uint8_t GLB_RTCnow=0;
#endif
float GLB_RTCtemp=0.0;



bool GLBsingingQuickly=false;


#ifdef LAMP_ENABLE_LSLIB
#define PIN_LED_STRIP             4

#define LSdistanceZero     0xEE00FD  
#define LSdistanceZeroT1   0xFF0000
#define LSdistanceZeroT2   0x00FF00
#define LSdistanceZeroIdle 0xFFFFFF
#define LSdistanceA        0x0000FF
#define LSdistanceAQuickly 0x0000FF
#define LSsinging          0xFFFFFF


#define LStempVeryCold   0x0000FF
#define LStempCold       0x5050FF
#define LStempOK         0x468282
#define LStempWarm       0xFBD971
#define LStempVeryWarm   0xFBA022
#define LStempHot        0xFF3030
#define LStempVeryHot    0xFF0000


#define LStempVeryColdZ   0x000060
#define LStempColdZ       0x181860
#define LStempOKZ         0x183030
#define LStempWarmZ       0x605025
#define LStempVeryWarmZ   0x605008
#define LStempHotZ        0x601212
#define LStempVeryHotZ    0x600000


#define NUM_LEDS 3
CRGB GLBledStripBufferMem[NUM_LEDS];
LSEM GLBledStrip(GLBledStripBufferMem,NUM_LEDS);
#endif 

//unsigned long time;
SimpleTimer mainTimers;

//--------------- SONGS ------------------------
const char  mario[] PROGMEM = {"mario:d=4,o=5,b=100:16e6,16e6,32p,8e6,16c6,8e6,8g6,8p,8g,8p,8c6,16p,8g,16p,8e,16p,8a,8b,16a#,8a,16g.,16e6,16g6,8a6,16f6,8g6,8e6,16c6,16d6,8b,16p,8c6,16p,8g,16p,8e,16p,8a,8b,16a#,8a,16g.,16e6,16g6,8a6,16f6,8g6,8e6,16c6,16d6,8b,8p,16g6,16f#6,16f6,16d#6,16p,16e6,16p,16g#,16a,16c6,16p,16a,16c6,16d6,8p,16g6,16f#6,16f6,16d#6,16p,16e6,16p,16c7,16p,16c7,16c7,p,16g6,16f#6,16f6,16d#6,16p,16e6,16p,16g#,16a,16c6,16p,16a,16c6,16d6,8p,16d#6,8p,16d6,8p,16c6\0"};

const char  mi[] PROGMEM = {"MissionImp:d=16,o=6,b=95:32d,32d#,32d,32d#,32d,32d#,32d,32d#,32d,32d,32d#,32e,32f,32f#,32g,g,8p,g,8p,a#,p,c7,p,g,8p,g,8p,f,p,f#,p,g,8p,g,8p,a#,p,c7,p,g,8p,g,8p,f,p,f#,p,a#,g,2d,32p,a#,g,2c#,32p,a#,g,2c,a#5,8c,2p,32p,a#5,g5,2f#,32p,a#5,g5,2f,32p,a#5,g5,2e,d#,8d\0"};

const char  StarWars[] PROGMEM = {"StarWars:d=4,o=5,b=45:32p,32f#,32f#,32f#,8b.,8f#.6,32e6,32d#6,32c#6,8b.6,16f#.6,32e6,32d#6,32c#6,8b.6,16f#.6,32e6,32d#6,32e6,8c#.6,32f#,32f#,32f#,8b.,8f#.6,32e6,32d#6,32c#6,8b.6,16f#.6,32e6,32d#6,32c#6,8b.6,16f#.6,32e6,32d#6,32e6,8c#6\0"};

const char  Imperial[] PROGMEM = {"Imperial:d=4,o=5,b=100:e,e,e,8c,16p,16g,e,8c,16p,16g,e,p,b,b,b,8c6,16p,16g,d#,8c,16p,16g,e,8p\0"};

const char  indiana[] PROGMEM = {"Indiana:d=4,o=5,b=250:e,8p,8f,8g,8p,1c6,8p.,d,8p,8e,1f,p.,g,8p,8a,8b,8p,1f6,p,a,8p,8b,2c6,2d6,2e6,e,8p,8f,8g,8p,1c6,p,d6,8p,8e6,1f.6,g,8p,8g,e.6,8p,d6,8p,8g,e.6,8p,d6,8p,8g,f.6,8p,e6,8p,8d6,2c6\0"};

#ifdef LAMP_ENABLE_EXTRAMUSIC

const char  AbbaMammaMia[] PROGMEM = {"AbbaMammaMia:d=4,o=5,b=40:32f6,32d#6,32f6,32d#6,8p,32d#6,32d#6,32f6,32g6,32f6,32d#6,16p,16f6,32d#6,16p,16g#6,32g#6,32g#6,32g#6,16g6,16d#6,16p,32f6,32d#6,32f6,32d#6,8p,32d#6,32d#6,32f6,32g6,32f6,32d#6,16p,16f6,32d#6,16p,16g#6,32g#6,32g#6,32g#6,16g6,16d#6,16p,16a#.6,32a#6,32a#6,16a#6,16f6,16g6,16g#6,16p,32p,16g.6,32g6,32g6,16g6,16d6,16d#6,16f6,16p,32p,16f6,16d#6,32p,16g#6,32g#6,32g#6,32g#6,32g6,32d#6,32f6,32d#6\0"};

const char  Supercalif[] PROGMEM = {"Supercalif:d=32,o=5,b=45:f,g#,g#,g#,a#,g#,g#,f,g#,g#,a#,g#,16g#,16f#,g#,g#,g#,g#,a#,g#,g#,d#,g#,g#,a#,g#,16g#,16f,g#,g#,g#,g#,a#,g#,g#,g#,c#6,c#6,d#6,c#6,16c#6,16a#,a#,c#6,c6,a#,c#6,g#,g#,f,g#,a,a#,c6,16c#6,16c#6\0"};



const char  Twinkle[] PROGMEM = {"Twinkle:d=4,o=5,b=80:32p,8c,8c,8g,8g,8a,8a,g,8f,8f,8e,8e,8d,8d,c,8g,8g,8f,8f,8e,8e,d,8g,8g,8f,8f,8e,8e,d,8c,8c,8g,8g,8a,8a,g\0"};

#endif

#ifdef LAMP_ENABLE_EXTRAMUSIC
#define MAX_AUDIOS 8
#else
#define MAX_AUDIOS 5
#endif


#ifdef LAMP_ENABLE_RTTTL
RtttlTrackerItem  audios[MAX_AUDIOS];
RtttlTrackerList  audiosTracker;
#endif

#define DISTANCE_ZERO    12
#define DISTANCE_A       40
#define DISTANCE_A2      27


#define TIMEOUT_ALARM   120000
#define TIMEOUT_Z1      5000
int  GLB_timerMisc=-1;
bool GLB_timerMisc_expired=false;
#define TIMEOUT_IDLEZ   7000



#ifdef LAMP_ENABLE_ALARM
bool GLB_alarmON=false;   //TODO volatile information, add it to eprom
bool GLB_alarmExecuted=false;
//*-*-*-*-*-*-*-*-*-**-*-*-*-*-*-*-*-*-**-*-*-*-*-*-*-*-*-**-*-*-*-*-*-*-*-*-*
const uint8_t GLB_alarmHH=07;
const uint8_t GLB_alarmMM=45;
char    GLB_alarmStr[]="  :  " ;  //Please update this also
//*-*-*-*-*-*-*-*-*-**-*-*-*-*-*-*-*-*-**-*-*-*-*-*-*-*-*-**-*-*-*-*-*-*-*-*-*
#endif

//------------------------------------------------
//--- GLOBAL FUNCTIONS ---------------------------
//------------------------------------------------
#ifdef LAMP_ENABLE_DISPLAY
void display_idle();
void display_all();
#ifdef LAMP_ENABLE_ALARM
void display_alarm();
void display_setupAlarm();
#endif
#endif 

bool isItAlarmTime();

void STATE_idle();
void STATE_idleZ();
void STATE_distanceZero();
void STATE_distanceA();
void STATE_singing();

void goto_idle(int d);

// State pointer function
void (*GLBptrStateFunc)();


//------------------------------------------------
//-------    TIMER CALLBACKS     -----------------
//------------------------------------------------


//------------------------------------------------
#ifdef LAMP_DEBUG_STILLALIVE
void GLBcallbackLogging(void)
{
  Serial.println(F("DEBUG: still alive from timer logging..."));
}
#endif
//------------------------------------------------
#ifdef LAMP_ENABLE_DHT
#ifdef LAMP_DEBUG_DHT
void GLBcallbackLoggingTemp(void)
{
  Serial.println(F("DEBUG: Temp..."));
  Serial.print(" DTH: Temperature:");
  Serial.print(GLBsensorDHTTemp);
  Serial.print(" Celsius. Humidity(%):");
  Serial.println(GLBsensorDHTHum);
}
#endif
#endif
//------------------------------------------------
#ifdef LAMP_DEBUG_LW
void GLBcallbackLoggingLedWhite(void)
{
  Serial.println(F("DEBUG: Led white..."));
  digitalWrite(PIN_LED_WHITE,!digitalRead(PIN_LED_WHITE));
}
#endif

//------------------------------------------------
#ifdef LAMP_ENABLE_RTC
#ifdef LAMP_DEBUG_RTC
void GLBcallbackLoggingRTC(void)
{
  Serial.println(F("DEBUG: RTC..."));

  char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
  DateTime now = rtc.now();

  Serial.print(now.year(), DEC);
  Serial.print('/');
  Serial.print(now.month(), DEC);
  Serial.print('/');
  Serial.print(now.day(), DEC);
  Serial.print("");
  Serial.print(daysOfTheWeek[now.dayOfTheWeek()]);
  Serial.print(") ");
  Serial.print(now.hour(), DEC);
  Serial.print(':');
  Serial.print(now.minute(), DEC);
  Serial.print(':');
  Serial.print(now.second(), DEC);
  Serial.println();

  Serial.print(" since midnight 1/1/1970 = ");
  Serial.print(now.unixtime());
  Serial.print("s = ");
  Serial.print(now.unixtime() / 86400L);
  Serial.println("GLB_HCdistance");

  // calculate a date which is 7 days and 30 seconds into the future
  DateTime future (now + TimeSpan(7,12,30,6));

  Serial.print(" now + 7d + 30s: ");
  Serial.print(future.year(), DEC);
  Serial.print('/');
  Serial.print(future.month(), DEC);
  Serial.print('/');
  Serial.print(future.day(), DEC);
  Serial.print(' ');
  Serial.print(future.hour(), DEC);
  Serial.print(':');
  Serial.print(future.minute(), DEC);
  Serial.print(':');
  Serial.print(future.second(), DEC);
  Serial.println();

  Serial.print("Temperature: ");
  Serial.print(rtc.getTemperature());
  Serial.println(" C");

  Serial.println();

#ifdef LAMP_ENABLE_DISPLAY
  display_all();
#endif

}
#endif
#endif
//------------------------------------------------

#ifdef LAMP_ENABLE_LSLIB
void GLBcallbackLoggingLedStrip(void)
{
  Serial.println(F("DEBUG: Led strip..."));
  GLBledStrip.setAllLeds(0xAABBCC);
}
#endif
//------------------------------------------------
#ifdef LAMP_ENABLE_HCSR04
#ifdef LAMP_DEBUG_HCSR04
void GLBcallbackLoggingUltrasonic(void)
{
  Serial.println(F("DEBUG: Ultrasonic..."));
  if(GLBsensorHC.isFinished()){
    Serial.print(F("  Distance CM:"));
    Serial.println(GLBsensorHC.getOngoingDistance());
    GLBsensorHC.trigger();  
  }
}
#endif
#endif
//------------------------------------------------
void GLBcallbackTimeoutMisc(void)
{
  if (GLB_timerMisc != -1)     
    mainTimers.deleteTimer(GLB_timerMisc);
  GLB_timerMisc=-1;
  GLB_timerMisc_expired=true;
}


//------------------------------------------------
void resetTimerMisc(void)
{
  GLBcallbackTimeoutMisc();
  GLB_timerMisc_expired=false;
}


//------------------------------------------------
void updateTempLSZ() { 

#ifdef LAMP_ENABLE_LSLIB
  CRGB ls=0;

  if      (GLBsensorDHTTemp < 15.0)  { ls=LStempVeryColdZ;} 
  else if (GLBsensorDHTTemp < 18.0)  { ls=LStempColdZ;} 
  else if (GLBsensorDHTTemp < 21.0)  { ls=LStempOKZ;} 
  else if (GLBsensorDHTTemp < 24.0)  { ls=LStempWarmZ;} 
  else if (GLBsensorDHTTemp < 27.0)  { ls=LStempVeryWarmZ;} 
  else if (GLBsensorDHTTemp < 30.0)  { ls=LStempHotZ;} 
  else                { ls=LStempVeryHotZ;} 

  GLBledStrip.setAllLeds(ls);
#endif
}
//------------------------------------------------
void updateTempLS() { 
#ifdef LAMP_ENABLE_LSLIB
  CRGB ls=0;

  if      (GLBsensorDHTTemp < 15.0)  { ls=LStempVeryCold;} 
  else if (GLBsensorDHTTemp < 18.0)  { ls=LStempCold;} 
  else if (GLBsensorDHTTemp < 21.0)  { ls=LStempOK;} 
  else if (GLBsensorDHTTemp < 24.0)  { ls=LStempWarm;} 
  else if (GLBsensorDHTTemp < 27.0)  { ls=LStempVeryWarm;} 
  else if (GLBsensorDHTTemp < 30.0)  { ls=LStempHot;} 
  else                { ls=LStempVeryHot;} 

  GLBledStrip.setAllLeds(ls);
#endif
}

//------------------------------------------------
//------------------------------------------------
//------------------------------------------------

void setup() { 
  // Serial to debug AND comunication protocolo with PI              
  Serial.begin(9600);
  Serial.println(F("BOOT"));


#ifdef LAMP_ENABLE_LSLIB
  FastLED.addLeds<WS2812B,PIN_LED_STRIP,GRB>(GLBledStripBufferMem,NUM_LEDS);
#endif

#ifdef LAMP_ENABLE_DHT
  Serial.println(F("B DHT22"));
  GLBsensorDHT.begin();
#endif

  pinMode(PIN_LED_WHITE,OUTPUT);

  Serial.println(F("B HC"));
#ifdef LAMP_ENABLE_HCSR04
  GLBsensorHC.setup();
  GLBsensorHC.trigger();  
#endif

#ifdef LAMP_DEBUG_STILLALIVE
  mainTimers.setInterval(2000,GLBcallbackLogging);
#endif 

#ifdef LAMP_DEBUG_LW
  mainTimers.setInterval(5000,GLBcallbackLoggingLedWhite);
#endif 

#ifdef LAMP_DEBUG_LS
  mainTimers.setInterval(10000,GLBcallbackLoggingLedStrip);
#endif 

#ifdef LAMP_ENABLE_HCSR04
#ifdef LAMP_DEBUG_HCSR04
  mainTimers.setInterval(1000,GLBcallbackLoggingUltrasonic);
#endif
#endif 

#ifdef LAMP_DEBUG_DHT
  mainTimers.setInterval(5000,GLBcallbackLoggingTemp);
#endif  

#ifdef LAMP_ENABLE_RTC
#ifdef LAMP_DEBUG_RTC
  mainTimers.setInterval(5000,GLBcallbackLoggingRTC);
#endif
#endif

  randomSeed(analogRead(0));

#ifdef LAMP_ENABLE_RTTTL
  Serial.println(F("B Tracker"));
  //memset(audios,0,sizeof(RtttlTrackerItem)*MAX_AUDIOS);
  audiosTracker.setup(audios,MAX_AUDIOS);
  audiosTracker.addItem(mario);
  audiosTracker.addItem(mi);
  audiosTracker.addItem(StarWars);
  audiosTracker.addItem(Imperial);
  audiosTracker.addItem(indiana);
#ifdef LAMP_ENABLE_EXTRAMUSIC
  audiosTracker.addItem(AbbaMammaMia);
  audiosTracker.addItem(Supercalif);
  audiosTracker.addItem(Twinkle);
#endif
  //REMINDER IF ADD MORE,MAKE SURE MAX_AUDIOS is sync
#endif

#ifdef LAMP_ENABLE_RTC
  Serial.println(F("B RTC"));
  if (! rtc.begin()) {
    Serial.print(F("F!"));
  }
  if (rtc.lostPower()) {
    Serial.print(F("RTC!"));
  }
#endif



#ifdef LAMP_ENABLE_DISPLAY
  Serial.println(F("B u8g2"));
  u8g2.begin(); 
#endif


  goto_idle(0);

  Serial.println(F("BD"));

}

//-------------------------------------------------
void goto_idle(int d)
{
#ifdef LAMP_DEBUG_STATES
  #ifdef LAMP_ENABLE_DISPLAY 
  u8g2.clearBuffer();
  #endif
  Serial.print(F("NST_idle"));
  Serial.println(d);
#endif


  resetTimerMisc();
  GLBptrStateFunc=STATE_idle; 
}

//-------------------------------------------------
void STATE_idle()
{
  if (isItAlarmTime()) return;
  
  if (GLB_HCdistance < DISTANCE_ZERO)            { 
#ifdef LAMP_DEBUG_STATES
    Serial.print(F("NST_distanceZero"));
    Serial.println(GLB_HCdistance);
#endif

#ifdef LAMP_ENABLE_RTTTL
    rtttl::stop(); 
#endif

#ifdef LAMP_ENABLE_LSLIB
    GLBledStrip.setAllLeds(LSdistanceZero);  
#endif
    GLBptrStateFunc=STATE_distanceZero; 
    return;
  }  


  if (GLB_HCdistance > DISTANCE_ZERO   && GLB_HCdistance < DISTANCE_A)  { 
#ifdef LAMP_DEBUG_STATES
    Serial.print(F("NST_distanceA"));
    Serial.println(GLB_HCdistance);
#endif

#ifdef LAMP_ENABLE_RTTTL
    rtttl::beginF(PIN_BUZZER,audiosTracker.getOrderedItem(true));  
#endif
#ifdef LAMP_ENABLE_LSLIB
    GLBledStrip.setAllLeds(LSdistanceA); 
#endif
    GLBptrStateFunc=STATE_distanceA;
    return;
  }

  #ifdef LAMP_ENABLE_DISPLAY
  display_idle();
  #endif
  updateTempLS();

  if (GLB_timerMisc == -1){
    GLB_timerMisc=mainTimers.setTimeout(TIMEOUT_IDLEZ,GLBcallbackTimeoutMisc);
  }

  if (GLB_timerMisc_expired){
    GLB_timerMisc_expired=false;

#ifdef LAMP_DEBUG_STATES
    Serial.print(F("NST_idleZ"));
    Serial.println(GLB_HCdistance);
#endif
    resetTimerMisc();
    GLBptrStateFunc=STATE_idleZ; 
    return;
  }
}

//-------------------------------------------------
void STATE_idleZ()
{
  if (isItAlarmTime()) return;

  if (GLB_HCdistance < DISTANCE_A)            { 
    //WAKE UP MY FRIEND! GO TO IDLE, THERE YOUR WAKE UP PROPERLY
    goto_idle(GLB_HCdistance); return;
  } 

  //WHITE LED TURN OFF
  digitalWrite(PIN_LED_WHITE,LOW);

  #ifdef LAMP_ENABLE_DISPLAY
  display_idle();
  #endif
  updateTempLSZ();

}


//-------------------------------------------------
void STATE_distanceZero()
{

  if (isItAlarmTime()) return;

#ifdef LAMP_ENABLE_RTTTL
  rtttl::stop(); //Just in case,silence pls
#endif 

  if (GLB_HCdistance > DISTANCE_ZERO)            { 
    goto_idle(GLB_HCdistance); return;
  } 

  if (GLB_timerMisc == -1){
    GLB_timerMisc=mainTimers.setTimeout(TIMEOUT_Z1,GLBcallbackTimeoutMisc);
    #ifdef LAMP_ENABLE_DISPLAY 
    u8g2.clearBuffer();
    #endif
    return;
  }
  if (GLB_timerMisc_expired){
    GLB_timerMisc_expired=false;
#ifdef LAMP_ENABLE_ALARM
    if (GLB_alarmON) {
        GLB_alarmON=false;  
#ifdef LAMP_ENABLE_RTTTL
        tone(PIN_BUZZER,500,100); 
#endif
    }
    else{
        GLB_alarmON=true;
#ifdef LAMP_ENABLE_RTTTL
        tone(PIN_BUZZER,1000,100); 
#endif
    }

#ifdef LAMP_ENABLE_DISPLAY
    display_setupAlarm();
#endif

#ifdef LAMP_DEBUG_STATES
    Serial.print(F("GLB_alarmON "));
    Serial.println(GLB_alarmON);
#endif
#endif
    goto_idle(GLB_HCdistance); return;

  }
  //else keep in this state and be happy
  //TODO show something different in display

  #ifdef LAMP_ENABLE_DISPLAY
  display_all();
  #endif
  return;

}


//-------------------------------------------------
void STATE_distanceA()
{
  if (isItAlarmTime()) return;

  if (GLB_HCdistance < DISTANCE_ZERO)            { 
#ifdef LAMP_DEBUG_STATES
    Serial.print(F("NST_distanceZero"));
    Serial.println(GLB_HCdistance);
#endif
#ifdef LAMP_ENABLE_RTTTL
    rtttl::stop(); 
#endif
#ifdef LAMP_ENABLE_LSLIB
    GLBledStrip.setAllLeds(LSdistanceZero);  
#endif
    GLBptrStateFunc=STATE_distanceZero; 
    return;
  } 

#ifdef LAMP_ENABLE_RTTTL
  if (GLB_HCdistance < DISTANCE_A) {
    if (!rtttl::isPlaying()) {  //Sing another
      rtttl::beginF(PIN_BUZZER,audiosTracker.getOrderedItem(true));  
    }
    //Tune speed
    bool bk=GLBsingingQuickly;
    if (GLB_HCdistance < DISTANCE_A2) {GLBsingingQuickly=false;}
    else                 {GLBsingingQuickly=true;}

    if (bk!=GLBsingingQuickly){
      if (GLB_HCdistance < DISTANCE_A2) {
        rtttl::changeSpeed(1);     
#ifdef LAMP_ENABLE_LSLIB
        GLBledStrip.setAllLeds(LSdistanceA);  
#endif
      }
      else {
        rtttl::changeSpeed(2);     
#ifdef LAMP_ENABLE_LSLIB
        GLBledStrip.setAllLeds(LSdistanceAQuickly);  
#endif
      }
    }
  

    return;
  }
#endif


#ifdef LAMP_ENABLE_RTTTL

  //Here only enters if distance is above A!!, let's song finish in other state
  //It should be playing, just in case checking it....
  if (rtttl::isPlaying()) {
#ifdef LAMP_DEBUG_STATES
    Serial.print(F("NST_singing"));
    Serial.println(GLB_HCdistance);
#endif

#ifdef LAMP_ENABLE_LSLIB
    GLBledStrip.setAllLeds(LSsinging);  
#endif
    GLBptrStateFunc=STATE_singing;
    return;
  }
#endif
  //Bizarre, no playing an distance above A, go to idle
  
    goto_idle(GLB_HCdistance); return;

}

//-------------------------------------------------
void STATE_singing()
{

  if (isItAlarmTime()) return;

#ifdef LAMP_ENABLE_RTTTL
  if (GLB_HCdistance < DISTANCE_ZERO)            { 
#ifdef LAMP_DEBUG_STATES
    Serial.print(F("NST_distanceZero"));
    Serial.println(GLB_HCdistance);
#endif

    rtttl::stop(); 
#ifdef LAMP_ENABLE_LSLIB
    GLBledStrip.setAllLeds(LSdistanceZero);  
#endif
    GLBptrStateFunc=STATE_distanceZero; 
    return;
  } 
  if (GLB_HCdistance < DISTANCE_A) {
#ifdef LAMP_DEBUG_STATES
    Serial.print(F("NST_distanceA"));
    Serial.println(GLB_HCdistance);
#endif

    // Changing  the song !!!!!!!!!! 
    rtttl::beginF(PIN_BUZZER,audiosTracker.getOrderedItem(true)); 
#ifdef LAMP_ENABLE_LSLIB 
    GLBledStrip.setAllLeds(LSdistanceA);  
#endif
    GLBptrStateFunc=STATE_distanceA;
    return;
  }

  if (!rtttl::isPlaying()) {
    goto_idle(GLB_HCdistance); return;
  }
#endif
}


#ifdef LAMP_ENABLE_ALARM
//-------------------------------------------------
void STATE_alarm()
{

  if (GLB_timerMisc == -1){
    GLB_timerMisc=mainTimers.setTimeout(TIMEOUT_ALARM,GLBcallbackTimeoutMisc);
    #ifdef LAMP_ENABLE_DISPLAY 
    u8g2.clearBuffer();
    #endif
    return;
  }
  if (GLB_timerMisc_expired){
    GLB_timerMisc_expired=false;
    GLB_alarmExecuted=true;
    goto_idle(GLB_HCdistance); return;
  }


  if (GLB_HCdistance < DISTANCE_A)            { 
    GLB_alarmExecuted=true;
    goto_idle(GLB_HCdistance); return;
  } 



#ifdef LAMP_ENABLE_RTTTL
  if (!rtttl::isPlaying()) {
    // Changing  the song !!!!!!!!!! 
    rtttl::beginF(PIN_BUZZER,audiosTracker.getOrderedItem(true)); 
  }
#endif

#ifdef LAMP_ENABLE_DISPLAY
  display_alarm();
#endif
}
#endif

//---------------------------------------------------------------------------

bool isItAlarmTime()
{ 
#ifdef LAMP_ENABLE_ALARM
  if (GLB_alarmON){
    if ((GLB_RTCnow.hour()==GLB_alarmHH) && (GLB_RTCnow.minute()==GLB_alarmMM)){
      if (!GLB_alarmExecuted){

        //Whatever state we should to go to ALARM!
        #ifdef LAMP_DEBUG_STATES
        Serial.print(F("NST_alarm"));
        #endif
        GLBptrStateFunc=STATE_alarm; 
        #ifdef LAMP_ENABLE_RTTTL
        rtttl::beginF(PIN_BUZZER,audiosTracker.getOrderedItem(true)); 
        #endif
        resetTimerMisc();
        return true;
      }
    } 
    else GLB_alarmExecuted=false;
  }
#endif 
  return false;
}


//-------------------------------------------------------
#ifdef LAMP_ENABLE_DISPLAY 
char GLBstrTime[9]; //here to avoid stack overflow

void updateGLBstrTime(bool sec=false)
{
#ifdef LAMP_ENABLE_RTC
  GLBstrTime[5]     = 0;
  if (sec) {
    GLBstrTime[8]     = 0;
    GLBstrTime[7]     = GLB_RTCnow.second() % 10 + 48;
    GLBstrTime[6]     = GLB_RTCnow.second() / 10 + 48;
    GLBstrTime[5]     = ':';
  }
  GLBstrTime[4]     = GLB_RTCnow.minute() % 10 + 48;
  GLBstrTime[3]     = GLB_RTCnow.minute() / 10 + 48;
  GLBstrTime[2]     = ':';
  GLBstrTime[1]     = GLB_RTCnow.hour()   % 10 + 48;
  GLBstrTime[0]     = GLB_RTCnow.hour()   / 10 + 48;
#else
  GLBstrTime[0]     = '-';
  GLBstrTime[1]     = 0;
#endif

}

#ifdef LAMP_ENABLE_ALARM
void updateGLB_alarmStr(bool sec=false)
{
  GLB_alarmStr[4]     = GLB_alarmMM % 10 + 48;
  GLB_alarmStr[3]     = GLB_alarmMM / 10 + 48;
  GLB_alarmStr[1]     = GLB_alarmHH   % 10 + 48;
  GLB_alarmStr[0]     = GLB_alarmHH   / 10 + 48;
}
#endif

bool GLBidleShowTime=false;
void display_idle(){
  bool bk=GLBidleShowTime;
  uint8_t animation=0;
  
  GLBidleShowTime=false;
  if (GLB_RTCnow.second() < 5){
    GLBidleShowTime=true;
    animation=GLB_RTCnow.second()+1;
  }
  else if  (GLB_RTCnow.second() >=30  && GLB_RTCnow.second() < 35){
    GLBidleShowTime=true;
    animation=(GLB_RTCnow.second()-30)+1;
  }


  if (bk != GLBidleShowTime)   u8g2.clearBuffer();

  updateGLBstrTime();

  u8g2.firstPage();
  do {
    if (GLBidleShowTime) {
      u8g2.setCursor(4,32);
      u8g2.setFont(u8g2_font_logisoso32_tn);
      u8g2.print(GLBstrTime);    
      for (uint8_t i=0;i<animation;i++)
        u8g2.drawCircle(110, 16,i*2, U8G2_DRAW_ALL);
   
    }
    else {
      u8g2.drawVLine(48, 2, 30);

      u8g2.setCursor(0,14);
      u8g2.setFont(u8g2_font_helvB12_tr);
      u8g2.print(GLBstrTime);   

      u8g2.setCursor(0,32);
      u8g2.print(GLBsensorDHTHum,1);
      u8g2.print("%");

      u8g2.setFont(u8g2_font_logisoso32_tn);
      u8g2.setCursor(50,32);
      u8g2.print(GLBsensorDHTTemp,1);
      u8g2.drawCircle(125, 5, 2, U8G2_DRAW_ALL);
   }
   if (GLB_alarmON) {
      u8g2.setCursor(118,32);
      u8g2.setFont(u8g2_font_helvB12_tr);
      u8g2.print('A');   
   }
  } while ( u8g2.nextPage() );



}

void display_all(){

  u8g2.firstPage();
  do {
    u8g2.drawVLine(64, 2, 30);

    u8g2.setCursor(0,14);
    u8g2.setFont(u8g2_font_helvB12_tr);
    updateGLBstrTime(true);
    u8g2.print(GLBstrTime);  

#ifdef LAMP_ENABLE_RTC
    u8g2.setCursor(10,32);
    u8g2.print(GLB_RTCtemp,1);
    u8g2.print(" C");
#endif

    u8g2.setCursor(70,14);
    u8g2.print(GLBsensorDHTTemp,1);
    u8g2.print(" C");

    u8g2.setCursor(70,32);
    u8g2.print(GLBsensorDHTHum,1);
    u8g2.print(" %");

  } while ( u8g2.nextPage() );
}


//-----------------------------------------------------------------
#ifdef LAMP_ENABLE_ALARM
uint8_t GLBdisplay_alarm_state=0;
void display_alarm()
{
  uint8_t bk=GLBdisplay_alarm_state;
  if (GLB_RTCnow.second() % 3==0){
    GLBdisplay_alarm_state=0;
  }
  else
  {
    GLBdisplay_alarm_state=1;
  }

  if (bk!=GLBdisplay_alarm_state){
    u8g2.clearBuffer();
  }
  u8g2.firstPage();
  updateGLBstrTime(false);
  do {  
    if (GLBdisplay_alarm_state){
      u8g2.setCursor(4,32);
      u8g2.setFont(u8g2_font_logisoso32_tn);
      u8g2.print(GLBstrTime);  
    }
    else{
      u8g2.setCursor(45,32);
      u8g2.setFont(u8g2_font_open_iconic_embedded_4x_t);
      u8g2.print('\x41');
    }
  } while ( u8g2.nextPage() );
}
//---------------------------------------------------------------
void display_setupAlarm()
{
  u8g2.clearBuffer();
  updateGLB_alarmStr();
  u8g2.firstPage();
  do {  
    u8g2.setCursor(2,32);
    u8g2.setFont(u8g2_font_open_iconic_embedded_4x_t);
    if (GLB_alarmON)    u8g2.print('\x41');
    else                u8g2.print('\x4D');

    u8g2.setFont(u8g2_font_helvB12_tr);
    u8g2.setCursor(30,14);
    u8g2.print("ALARM ");
    if (GLB_alarmON)    u8g2.print("ON");
    else                u8g2.print("OFF");
    u8g2.setCursor(60,32);
    u8g2.print(GLB_alarmStr);
  } while ( u8g2.nextPage() );

  delay(2000);
}
#endif
#endif


//-------------------------------------------------
//-------------------------------------------------
//-------------------------------------------------
void loop() { 
#ifdef LAMP_DEBUG_LOOP  
  Serial.print("+");
#endif
 
  //------------- INPUT REFRESHING ----------------
  // Let use ugly global variables in those costly sensors to save ram...

#ifdef LAMP_ENABLE_HCSR04
#ifdef LAMP_DEBUG_LOOP  
  Serial.print("H");
#endif
  GLBsensorHC.refresh();
#endif

#ifdef LAMP_ENABLE_DHT
  GLBsensorDHTTemp = GLBsensorDHT.readTemperature();
  GLBsensorDHTHum  = GLBsensorDHT.readHumidity();
//  GLBsensorDHTTemp = 25; //DEBUG FAKE
//  GLBsensorDHTHum  = 44; //DEBUG FAKE 

#endif

#ifdef LAMP_ENABLE_RTC
  GLB_RTCnow=rtc.now();
  GLB_RTCtemp=rtc.getTemperature();
#endif

#ifdef LAMP_ENABLE_HCSR04
  GLB_HCdistance=GLBsensorHC.getLatestDistanceMedian();
//  GLB_HCdistance=333; //DEBUG FAKE
#endif
    
  //--------- TIME TO THINK MY FRIEND -------------
  // State machine as main controller execution
#ifdef LAMP_DEBUG_LOOP  
  Serial.print("S");
#endif
  GLBptrStateFunc();
#ifdef LAMP_DEBUG_LOOP  
  Serial.print("R");
#endif
  mainTimers.run();

  // ------------- OUTPUT REFRESHING ---------------
#ifdef LAMP_ENABLE_LSLIB
#ifdef LAMP_DEBUG_LOOP  
  Serial.print("L");
#endif
  //GLBledStrip.refresh();
  FastLED.show();
#endif

#ifdef LAMP_ENABLE_RTTTL
#ifdef LAMP_DEBUG_LOOP  
  Serial.print("R");
#endif
  rtttl::play();
#endif
}

