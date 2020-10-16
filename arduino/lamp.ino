#include <stdio.h>
#ifdef LAMP_ENABLE_LSLIB
#include "lsem.h"
#endif
#include "LowPower.h"
#include "SimpleTimer.h"
#include "DHTsensor.h"

#include "HCSR04sensor.h"
#include "NonBlockingRtttl.h"
#include "RtttlTrackerList.h"
#include <Arduino.h>
#include <U8g2lib.h>
#include <Wire.h>

#include "RTClib.h"




//------------------------------------------------
//--- DEBUG  FLAGS     ---------------------------
//------------------------------------------------
// Check makefile

//------------------------------------------------
//--- GLOBAL VARIABLES ---------------------------
//------------------------------------------------


// PINS 

const int PIN_SENSOR_HCSR04_TRIGGER =3; 
const int PIN_SENSOR_HCSR04_ECHO    =2;  //INT NEEDED
const int PIN_LED_STRIP             =4;
const int PIN_LED_WHITE             =5; 
const int PIN_SENSOR_TEMP           =6;
const int PIN_BUZZER                =8;

// INTERRUPTS
const int INT_SENSOR_HCSR04         =0;

#define MEDIAN_FILTER_SIZE_SENSOR_HCSR04  15

// Sensors
DHTsensor    GLBsensorDHT;
HCSR04sensor GLBsensorHC(PIN_SENSOR_HCSR04_TRIGGER,PIN_SENSOR_HCSR04_ECHO,INT_SENSOR_HCSR04,MEDIAN_FILTER_SIZE_SENSOR_HCSR04);

// Display
U8G2_SSD1306_128X32_UNIVISION_1_SW_I2C u8g2(U8G2_R0, /* clock=*/ SCL, /* data=*/ SDA, /* reset=*/ U8X8_PIN_NONE);

// RTC
RTC_DS3231 rtc;


// Misc 
unsigned int aliveLoopCounter=0;
unsigned int msCounter=0;
unsigned int sCounter=0;
unsigned int latestTempReported=0;

bool singingQuickly=false;



#ifdef LAMP_ENABLE_SERIAL_INPUT
char    GLBauxString[100];  
char    GLBserialInputString[100]; // a string to hold incoming data
int     GLBserialIx=0;
bool    GLBserialInputStringReady = false; // whether the string is complete
#endif

#ifdef LAMP_ENABLE_LSLIB
const char inputTest[]  PROGMEM = {":LP0200:LT0050:LMN:LQ:LMA:LP0100:LCFF,00,00:LQ:LMA:LP0100:LC00,FF,00:LQ:LMA:LP0100:LC00,00,FF\0"};

const char LSwelcome[]          PROGMEM = {":LP0020:LT0005:LMK:LCFF,00,00:LQ:LMk:LQ:LMN\0"};
const char LSidle[]             PROGMEM = {":LX:LMN:LT0000:LP0100\0"};  //it will be override by temp
const char LSdistanceZero[]     PROGMEM = {":LX:LMA:LT0000:LP0070:LCEE,00,FD\0"};  
const char LSdistanceZeroT1[]   PROGMEM = {":LX:LMK:LT0000:LP0050:LCFF,00,00\0"};
const char LSdistanceZeroT2[]   PROGMEM = {":LX:LMK:LT0000:LP0050:LC00,FF,00\0"};
const char LSdistanceZeroIdle[] PROGMEM = {":LX:LMA:LT0000:LP0000:LCFF,FF,FF\0"};
const char LSdistanceA[]        PROGMEM = {":LX:LMK:LT0000:LP0250:LC00,00,FF\0"};
const char LSdistanceAQuickly[] PROGMEM = {":LX:LMK:LT0000:LP0050:LC00,00,FF\0"};
const char LSsinging[]          PROGMEM = {":LX:LMN:LT0000:LP0100\0"};


const char LStempVeryCold[]   PROGMEM = {":LX:LMA:LT0000:LP0000:LC00,00,FF\0"};
const char LStempCold[]       PROGMEM = {":LX:LMA:LT0000:LP0000:LC50,50,FF\0"};
const char LStempOK[]         PROGMEM = {":LX:LMA:LT0000:LP0000:LC46,82,82\0"};
const char LStempWarm[]       PROGMEM = {":LX:LMA:LT0000:LP0000:LCFB,D9,71\0"};
const char LStempVeryWarm[]   PROGMEM = {":LX:LMA:LT0000:LP0000:LCFB,A0,22\0"};
const char LStempHot[]        PROGMEM = {":LX:LMA:LT0000:LP0000:LCFF,30,30\0"};
const char LStempVeryHot[]    PROGMEM = {":LX:LMA:LT0000:LP0000:LCFF,00,00\0"};


const char LStempVeryColdZ[]   PROGMEM = {":LX:LMA:LT0000:LP0000:LC00,00,60\0"};
const char LStempColdZ[]       PROGMEM = {":LX:LMA:LT0000:LP0000:LC18,18,60\0"};
const char LStempOKZ[]         PROGMEM = {":LX:LMA:LT0000:LP0000:LC18,30,30\0"};
const char LStempWarmZ[]       PROGMEM = {":LX:LMA:LT0000:LP0000:LC60,50,25\0"};
const char LStempVeryWarmZ[]   PROGMEM = {":LX:LMA:LT0000:LP0000:LC60,50,08\0"};
const char LStempHotZ[]        PROGMEM = {":LX:LMA:LT0000:LP0000:LC60,12,12\0"};
const char LStempVeryHotZ[]    PROGMEM = {":LX:LMA:LT0000:LP0000:LC60,00,00\0"};

/*
const char LStempVeryColdZ[]   PROGMEM = {":LX:LMO01:LT0000:LP0000:LC00,00,80\0"};
const char LStempColdZ[]       PROGMEM = {":LX:LMO01:LT0000:LP0000:LC05,05,40\0"};
const char LStempOKZ[]         PROGMEM = {":LX:LMO01:LT0000:LP0000:LC05,10,10\0"};
const char LStempWarmZ[]       PROGMEM = {":LX:LMO01:LT0000:LP0000:LC10,10,10\0"};
const char LStempVeryWarmZ[]   PROGMEM = {":LX:LMO01:LT0000:LP0000:LC20,15,05\0"};;
const char LStempHotZ[]        PROGMEM = {":LX:LMO01:LT0000:LP0000:LC20,12,02\0"};
const char LStempVeryHotZ[]    PROGMEM = {":LX:LMO01:LT0000:LP0000:LC20,02,02\0"};
*/

const char *GLB_lsTempBK=0;

#define NUM_LEDS 3
CRGB GLBledStripBufferMem[NUM_LEDS];

//--------- Global functions declarations ------------
void GLBcallbackTimeoutLS(void);
void GLBcallbackPauseLS(void);

LSEM GLBledStrip(GLBledStripBufferMem,NUM_LEDS,GLBcallbackPauseLS,GLBcallbackTimeoutLS);
#endif 

//unsigned long time;
SimpleTimer mainTimers;

//--------------- SONGS ------------------------
const char  mario[] PROGMEM = {"mario:d=4,o=5,b=100:16e6,16e6,32p,8e6,16c6,8e6,8g6,8p,8g,8p,8c6,16p,8g,16p,8e,16p,8a,8b,16a#,8a,16g.,16e6,16g6,8a6,16f6,8g6,8e6,16c6,16d6,8b,16p,8c6,16p,8g,16p,8e,16p,8a,8b,16a#,8a,16g.,16e6,16g6,8a6,16f6,8g6,8e6,16c6,16d6,8b,8p,16g6,16f#6,16f6,16d#6,16p,16e6,16p,16g#,16a,16c6,16p,16a,16c6,16d6,8p,16g6,16f#6,16f6,16d#6,16p,16e6,16p,16c7,16p,16c7,16c7,p,16g6,16f#6,16f6,16d#6,16p,16e6,16p,16g#,16a,16c6,16p,16a,16c6,16d6,8p,16d#6,8p,16d6,8p,16c6\0"};

const char  mi[] PROGMEM = {"MissionImp:d=16,o=6,b=95:32d,32d#,32d,32d#,32d,32d#,32d,32d#,32d,32d,32d#,32e,32f,32f#,32g,g,8p,g,8p,a#,p,c7,p,g,8p,g,8p,f,p,f#,p,g,8p,g,8p,a#,p,c7,p,g,8p,g,8p,f,p,f#,p,a#,g,2d,32p,a#,g,2c#,32p,a#,g,2c,a#5,8c,2p,32p,a#5,g5,2f#,32p,a#5,g5,2f,32p,a#5,g5,2e,d#,8d\0"};
/*
const char  indiana[] PROGMEM = {"Indiana:d=4,o=5,b=250:e,8p,8f,8g,8p,1c6,8p.,d,8p,8e,1f,p.,g,8p,8a,8b,8p,1f6,p,a,8p,8b,2c6,2d6,2e6,e,8p,8f,8g,8p,1c6,p,d6,8p,8e6,1f.6,g,8p,8g,e.6,8p,d6,8p,8g,e.6,8p,d6,8p,8g,f.6,8p,e6,8p,8d6,2c6\0"};


const char  StarWars[] PROGMEM = {"StarWars:d=4,o=5,b=45:32p,32f#,32f#,32f#,8b.,8f#.6,32e6,32d#6,32c#6,8b.6,16f#.6,32e6,32d#6,32c#6,8b.6,16f#.6,32e6,32d#6,32e6,8c#.6,32f#,32f#,32f#,8b.,8f#.6,32e6,32d#6,32c#6,8b.6,16f#.6,32e6,32d#6,32c#6,8b.6,16f#.6,32e6,32d#6,32e6,8c#6\0"};

const char  AbbaMammaMia[] PROGMEM = {"AbbaMammaMia:d=4,o=5,b=40:32f6,32d#6,32f6,32d#6,8p,32d#6,32d#6,32f6,32g6,32f6,32d#6,16p,16f6,32d#6,16p,16g#6,32g#6,32g#6,32g#6,16g6,16d#6,16p,32f6,32d#6,32f6,32d#6,8p,32d#6,32d#6,32f6,32g6,32f6,32d#6,16p,16f6,32d#6,16p,16g#6,32g#6,32g#6,32g#6,16g6,16d#6,16p,16a#.6,32a#6,32a#6,16a#6,16f6,16g6,16g#6,16p,32p,16g.6,32g6,32g6,16g6,16d6,16d#6,16f6,16p,32p,16f6,16d#6,32p,16g#6,32g#6,32g#6,32g#6,32g6,32d#6,32f6,32d#6\0"};


const char  Imperial[] PROGMEM = {"Imperial:d=4,o=5,b=100:e,e,e,8c,16p,16g,e,8c,16p,16g,e,p,b,b,b,8c6,16p,16g,d#,8c,16p,16g,e,8p\0"};



const char  Supercalif[] PROGMEM = {"Supercalif:d=32,o=5,b=45:f,g#,g#,g#,a#,g#,g#,f,g#,g#,a#,g#,16g#,16f#,g#,g#,g#,g#,a#,g#,g#,d#,g#,g#,a#,g#,16g#,16f,g#,g#,g#,g#,a#,g#,g#,g#,c#6,c#6,d#6,c#6,16c#6,16a#,a#,c#6,c6,a#,c#6,g#,g#,f,g#,a,a#,c6,16c#6,16c#6\0"};



const char  Twinkle[] PROGMEM = {"Twinkle:d=4,o=5,b=80:32p,8c,8c,8g,8g,8a,8a,g,8f,8f,8e,8e,8d,8d,c,8g,8g,8f,8f,8e,8e,d,8g,8g,8f,8f,8e,8e,d,8c,8c,8g,8g,8a,8a,g\0"};
*/

const char warmingUp[] PROGMEM ={"TwinkleShort:d=4,o=5,b=80:32p,8c,8c,8g,8g,8a,8a,g\0"};



#define MAX_AUDIOS 2
/* ALTERNATIVE WITHOUT TRACKING 
const char *audiosTable[MAX_AUDIOS]= {
  mario,mi,indiana
};
*/

RtttlTrackerItem  audios[MAX_AUDIOS];
RtttlTrackerList  audiosTracker;


#define DISTANCE_ZERO    12
#define DISTANCE_A       40
#define DISTANCE_A2      27


#define TIMEOUT_Z1  3500
#define TIMEOUT_Z2  6500  // it covers singing temp
int GLB_timerZ1=-1;
bool GLB_timerZ1_expired=false;

#define TIMEOUT_IDLEZ  5000  // it covers singing temp
int GLB_timerIdleZ=-1;
bool GLB_timerIdleZ_expired=false;



#ifdef LAMP_DEBUG_TEMPCOLOR
#define TIMEOUT_TEMPCOLOR  2000
int GLB_timerTempcolor=-1;
bool GLB_timerTempcolor_expired=false;
float GLB_Tempcolor=11.2;
#endif 


//------------------------------------------------
//--- GLOBAL FUNCTIONS ---------------------------
//------------------------------------------------


//-- SOME NEEDED PROTOTYPES ---------------------------

void STATE_init(void);
void STATE_welcome(void);
void STATE_idle(void);
void STATE_idleZ(void);
#ifdef LAMP_ENABLE_SERIAL_INPUT
void STATE_LDcmd(void);
#endif
void STATE_distanceZero(void);
void STATE_distanceZero_T1(void);
void STATE_distanceZero_T2(void);
void STATE_distanceZero_idle(void);
void STATE_distanceA(void);
void STATE_singing(void);

// State pointer function
void (*GLBptrStateFunc)();



//------------------------------------------------
//-------    TIMER CALLBACKS     -----------------
//------------------------------------------------
#ifdef LAMP_ENABLE_LSLIB
void GLBcallbackTimeoutLS(void)
{
  //if (LSEM.getDebug()) {Serial.println(F("DEBUG: callbackTimeout"));}
  GLBledStrip.callbackTimeout();
}
//------------------------------------------------
void GLBcallbackPauseLS(void)
{
  //Serial.println(F("DEBUG: callbackPause");
  GLBledStrip.callbackPause();
}
#endif

//------------------------------------------------
void GLBcallbackLogging(void)
{
  Serial.println(F("DEBUG: still alive from timer logging..."));
}
//------------------------------------------------
void GLBcallbackLoggingTemp(void)
{
  Serial.println(F("DEBUG: Temp..."));
  float t=GLBsensorDHT.getTemperature();
  float h=GLBsensorDHT.getHumidity();
  Serial.print(" DTH: Temperature:");
  Serial.print(t);
  Serial.print(" Celsius. Humidity(%):");
  Serial.println(h);
}
//------------------------------------------------
void GLBcallbackLoggingLedWhite(void)
{
  Serial.println(F("DEBUG: Led white..."));
  digitalWrite(PIN_LED_WHITE,!digitalRead(PIN_LED_WHITE));
}

//------------------------------------------------
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
  Serial.print(" (");
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
  Serial.println("d");

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
}
//------------------------------------------------

#ifdef LAMP_ENABLE_LSLIB
void GLBcallbackLoggingLedStrip(void)
{
  Serial.println(F("DEBUG: Led strip..."));
  GLBledStrip.processCommands(inputTest,true);
}
#endif
//------------------------------------------------
void GLBcallbackLoggingUltrasonic(void)
{
  Serial.println(F("DEBUG: Ultrasonic..."));
  if(GLBsensorHC.isFinished()){
    Serial.print(F("  Distance CM:"));
    Serial.println(GLBsensorHC.getOngoingDistance());
    GLBsensorHC.trigger();  
  }
}
//------------------------------------------------
void GLBcallbackTimeoutZ1(void)
{
  if (GLB_timerZ1 != -1)     
    mainTimers.deleteTimer(GLB_timerZ1);
  GLB_timerZ1=-1;
  GLB_timerZ1_expired=true;
}

//------------------------------------------------
void GLBcallbackTimeoutIdleZ(void)
{
  if (GLB_timerIdleZ != -1)     
    mainTimers.deleteTimer(GLB_timerIdleZ);
  GLB_timerIdleZ=-1;
  GLB_timerIdleZ_expired=true;
}

#ifdef LAMP_DEBUG_TEMPCOLOR
//------------------------------------------------
void GLBcallbackTimeoutTempcolor(void)
{
  if (GLB_timerTempcolor != -1)     
    mainTimers.deleteTimer(GLB_timerTempcolor);
  GLB_timerTempcolor=-1;
  GLB_timerTempcolor_expired=true;
}

//------------------------------------------------
void resetTimerTempcolor(void)
{
  GLBcallbackTimeoutTempcolor();
  GLB_timerTempcolor_expired=false;
}
#endif



//------------------------------------------------
void resetTimerZ(void)
{
  GLBcallbackTimeoutZ1();
  GLB_timerZ1_expired=false;
}


//------------------------------------------------
void resetTimerIdleZ(void)
{
  GLBcallbackTimeoutIdleZ();
  GLB_timerIdleZ_expired=false;
}


//------------------------------------------------
void GLBsingHum()
{
  float h=GLBsensorDHT.getHumidity();
  rtttl::playNumber(PIN_BUZZER,h);  
}
//------------------------------------------------
void GLBsingTemp()
{
  float t=GLBsensorDHT.getTemperature();
  rtttl::playNumber(PIN_BUZZER,t);  
}


//------------------------------------------------
void updateTempLSZ(float t) { 

#ifdef LAMP_ENABLE_LSLIB
  const char *ls=0;

  if      (t < 15.0)  { ls=LStempVeryColdZ;} 
  else if (t < 18.0)  { ls=LStempColdZ;} 
  else if (t < 21.0)  { ls=LStempOKZ;} 
  else if (t < 24.0)  { ls=LStempWarmZ;} 
  else if (t < 27.0)  { ls=LStempVeryWarmZ;} 
  else if (t < 30.0)  { ls=LStempHotZ;} 
  else                { ls=LStempVeryHotZ;} 

  if (ls!=GLB_lsTempBK){
    GLBledStrip.processCommands(ls,true);
  } 
  GLB_lsTempBK=ls;
#endif
}
//------------------------------------------------
void updateTempLS(float t) { 
#ifdef LAMP_ENABLE_LSLIB
  const char *ls=0;

  if      (t < 15.0)  { ls=LStempVeryCold;} 
  else if (t < 18.0)  { ls=LStempCold;} 
  else if (t < 21.0)  { ls=LStempOK;} 
  else if (t < 24.0)  { ls=LStempWarm;} 
  else if (t < 27.0)  { ls=LStempVeryWarm;} 
  else if (t < 30.0)  { ls=LStempHot;} 
  else                { ls=LStempVeryHot;} 

  if (ls!=GLB_lsTempBK){
    GLBledStrip.processCommands(ls,true);
  } 
  GLB_lsTempBK=ls;
#endif
}

//------------------------------------------------
//------------------------------------------------
//------------------------------------------------

void setup() { 
  // Serial to debug AND comunication protocolo with PI              
  Serial.begin(9600);
  Serial.println(F("Setup... 'came on,be my baby,came on'"));

#ifdef LAMP_ENABLE_SERIAL_INPUT
  GLBserialInputString[0]=0;
#endif 
  GLBptrStateFunc = STATE_init;
  Serial.println(F("STATE INIT"));
#ifdef LAMP_ENABLE_LSLIB
  FastLED.addLeds<WS2812B,PIN_LED_STRIP,GRB>(GLBledStripBufferMem,NUM_LEDS);
#endif
  Serial.println("Setup DHT22 ");
  if (GLBsensorDHT.setup(PIN_SENSOR_TEMP)!=0) {
      Serial.print("Failing dht22 setup on pin:");
      Serial.println(PIN_SENSOR_TEMP);
  }
  pinMode(PIN_LED_WHITE,OUTPUT);
  GLBsensorHC.setup();
  GLBsensorHC.trigger();  
  /* TODO constant sensor testing 
  mainTimers.setInterval(2000,GLBcallbackLogging);



  mainTimers.setInterval(5000,GLBcallbackLoggingLedWhite);

  mainTimers.setInterval(10000,GLBcallbackLoggingLedStrip);
  mainTimers.setInterval(1000,GLBcallbackLoggingUltrasonic);
  mainTimers.setInterval(5000,GLBcallbackLoggingTemp);
  */


#ifdef LAMP_DEBUG_RTC
  mainTimers.setInterval(10000,GLBcallbackLoggingRTC);
#endif


#ifdef LAMP_ENABLE_LSLIB
  #ifdef LAMP_DEBUG_TEMPCOLOR
  GLBledStrip.setDebug(true);
  #endif
#endif
  randomSeed(analogRead(0));

  //memset(audios,0,sizeof(RtttlTrackerItem)*MAX_AUDIOS);
  audiosTracker.setup(audios,MAX_AUDIOS);
  audiosTracker.addItem(mario);
  audiosTracker.addItem(mi);
  /*audiosTracker.addItem(indiana);
  audiosTracker.addItem(StarWars);
  audiosTracker.addItem(AbbaMammaMia);
  audiosTracker.addItem(Imperial);
  audiosTracker.addItem(Supercalif);
  audiosTracker.addItem(Twinkle);*/
  //REMINDER IF ADD MORE,MAKE SURE MAX_AUDIOS is sync


  u8g2.begin(); 


  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
  }

  if (rtc.lostPower()) {
    Serial.println("RTC lost power, let's set the time!");
    // When time needs to be set on a new device, or after a power loss, the
    // following line sets the RTC to the date & time this sketch was compiled
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // This line sets the RTC with an explicit date & time, for example to set
    // January 21, 2014 at 3am you would call:
    // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
  }

}

//------------------------------------------------

#ifdef LAMP_ENABLE_SERIAL_INPUT

void serialEvent() {
  while (Serial.available()) {
     char inChar = (char)Serial.read();
     if (inChar < 0x20) {
       GLBserialInputStringReady = true;
       GLBserialInputString[GLBserialIx]=0;
       GLBserialIx=0;
       return;
     }
     GLBserialInputString[GLBserialIx++]=inChar;
  }
}
#endif

//-------------------------------------------------
#ifdef LAMP_ENABLE_SERIAL_INPUT
void processSerialInputString()
{
  strcpy(GLBauxString,GLBserialInputString);
  GLBserialInputString[0]=0;
  GLBserialInputStringReady = false;
#ifdef LAMP_ENABLE_LSLIB
  GLBledStrip.processCommands(GLBauxString,true);
#endif
}
#endif

//-------------------------------------------------
void STATE_init(void)
{
  #ifdef LAMP_DEBUG_TEMPCOLOR
  int aux=0;

  digitalWrite(PIN_LED_WHITE,LOW);
  if (GLB_timerTempcolor == -1){
    GLB_timerTempcolor=mainTimers.setTimeout(TIMEOUT_TEMPCOLOR,GLBcallbackTimeoutTempcolor);
  }

  if (GLB_timerTempcolor_expired){
    GLB_Tempcolor+=3;
    GLB_timerTempcolor_expired=false;

    if (GLB_Tempcolor < 33){
      Serial.print(F("LAMP_DEBUG_TEMPCOLOR "));
      Serial.println(GLB_Tempcolor);
      updateTempLS(GLB_Tempcolor);
    }
    else if (GLB_Tempcolor < 55) {
      Serial.print(F("LAMP_DEBUG_TEMPCOLOR_Z "));
      Serial.println(GLB_Tempcolor-7*3);
      updateTempLSZ(GLB_Tempcolor-7*3);
    }
    else {
      resetTimerTempcolor(); //will progress in next iteration
#ifdef LAMP_ENABLE_LSLIB
      GLBledStrip.setDebug(false);
#endif
      goto stdpath;     
    }
  }  
  return;
  #endif

stdpath:


  tone(PIN_BUZZER,1000,20);
  digitalWrite(PIN_LED_WHITE,HIGH);
  delay(500);
  digitalWrite(PIN_LED_WHITE,LOW);
  delay(500);
  digitalWrite(PIN_LED_WHITE,HIGH);

  #ifdef LAMP_DEBUG_STATES
  Serial.println(F("STATE INIT -> WELCOME"));
  #endif
  #ifdef LAMP_ENABLE_LSLIB
  GLBledStrip.processCommands(LSwelcome,true);
  #endif
  GLBptrStateFunc=STATE_welcome;
  rtttl::beginF(PIN_BUZZER,warmingUp);

}
//-------------------------------------------------
void STATE_welcome(void)
{
#ifdef LAMP_ENABLE_LSLIB
  if (GLBledStrip.isIdle() && !rtttl::isPlaying()) {
    GLB_lsTempBK=0;
#else
  if (!rtttl::isPlaying()) {
#endif
    //Force a pause TODO ADD A NEW STAGE AND NON-BLOCKING SING TEMP
    delay(1000);
    #ifdef LAMP_ENABLE_LSLIB
    GLBledStrip.processCommands(LSidle,true);  
    #endif 

    GLBptrStateFunc=STATE_idle;
#ifdef LAMP_DEBUG_STATES
    Serial.println(F("STATE WELCOME -> IDLE"));
#endif
  }
}
//-------------------------------------------------
void STATE_idle(void)
{
  int d=GLBsensorHC.getLatestDistanceMedian();
#ifdef LAMP_DEBUG_HCSR04_MEDIAN
  int d2=GLBsensorHC.getLatestDistanceRead();
#endif

#ifdef LAMP_ENABLE_SERIAL_INPUT
  if (GLBserialInputStringReady){
    processSerialInputString();
    GLBptrStateFunc=STATE_LDcmd;
#ifdef LAMP_DEBUG_STATES
    Serial.println(F("STATE IDLE -> CMD LS"));
#endif 
  }
#endif

  resetTimerZ();


  if (d < DISTANCE_ZERO)            { 
#ifdef LAMP_DEBUG_STATES
    Serial.print(F("--->NEW STATE: STATE_distanceZero ("));
    Serial.print(d);
#ifdef LAMP_DEBUG_HCSR04_MEDIAN
    Serial.print(F(") "));
    Serial.print(d2);
#endif
    Serial.println(".");
#endif

    rtttl::stop(); 
#ifdef LAMP_ENABLE_LSLIB
    GLBledStrip.processCommands(LSdistanceZero,true);  
#endif
    GLBptrStateFunc=STATE_distanceZero; 
  }  
  if (d > DISTANCE_ZERO   && d < DISTANCE_A)  { 
#ifdef LAMP_DEBUG_STATES
    Serial.print(F("--->NEW STATE: STATE_distanceA ("));
    Serial.print(d);
#ifdef LAMP_DEBUG_HCSR04_MEDIAN
    Serial.print(F(") "));
    Serial.print(d2);
#endif
    Serial.println(".");
#endif

    rtttl::beginF(PIN_BUZZER,audiosTracker.getOrderedItem(true));  
#ifdef LAMP_ENABLE_LSLIB
    GLBledStrip.processCommands(LSdistanceA,true); 
#endif
    GLBptrStateFunc=STATE_distanceA;
  }

  if (GLBptrStateFunc!=STATE_idle) return;
 
  float t=GLBsensorDHT.getTemperature();

  updateTempLS(t);


  if (GLB_timerIdleZ == -1){
    GLB_timerIdleZ=mainTimers.setTimeout(TIMEOUT_IDLEZ,GLBcallbackTimeoutIdleZ);
  }

  if (GLB_timerIdleZ_expired){
    GLB_timerIdleZ_expired=false;

#ifdef LAMP_DEBUG_STATES
    Serial.print(F("--->NEW STATE: STATE_idleZ ("));
    Serial.print(d);
#ifdef LAMP_DEBUG_HCSR04_MEDIAN
    Serial.print(F(") "));
    Serial.print(d2);
#endif
    Serial.println(".");
#endif

    resetTimerIdleZ();
    GLBptrStateFunc=STATE_idleZ; 
    return;
  }
}

//-------------------------------------------------
void STATE_idleZ(void)
{
 
  int d=GLBsensorHC.getLatestDistanceMedian();
#ifdef LAMP_DEBUG_HCSR04_MEDIAN
  int d2=GLBsensorHC.getLatestDistanceRead();
#endif

  if (d < DISTANCE_A)            { 
    //WAKE UP MY FRIEND! GO TO IDLE, THERE YOUR WAKE UP PROPERLY
#ifdef LAMP_DEBUG_STATES
    Serial.print(F("--->NEW STATE: STATE_idle ("));
    Serial.print(d);
#ifdef LAMP_DEBUG_HCSR04_MEDIAN
    Serial.print(F(") "));
    Serial.print(d2);
#endif
    Serial.println(".");
#endif
#ifdef LAMP_ENABLE_LSLIB
    GLBledStrip.processCommands(LSidle,true); 
    GLB_lsTempBK=0; 
#endif
    GLBptrStateFunc=STATE_idle; 
    return;
  } 

  //WHITE LED TURN OFF
  digitalWrite(PIN_LED_WHITE,LOW);
  //LOW INTENSITY IN LD
  float t=GLBsensorDHT.getTemperature();

  updateTempLSZ(t);



}


#ifdef LAMP_ENABLE_SERIAL_INPUT
//-------------------------------------------------
void STATE_LDcmd(void)
{
  if (GLBserialInputStringReady){
    processSerialInputString();
  }
  else if (GLBledStrip.isIdle()) {
#ifdef LAMP_ENABLE_LSLIB
    GLBledStrip.processCommands(LSidle,true);  
    GLB_lsTempBK=0;
#endif
    GLBptrStateFunc=STATE_idle;
#ifdef LAMP_DEBUG_STATES
    Serial.println(F("STATE LD CMD -> IDLE"));
#endif
  }
}
#endif

//-------------------------------------------------
void STATE_distanceZero(void)
{
 
  int d=GLBsensorHC.getLatestDistanceMedian();
#ifdef LAMP_DEBUG_HCSR04_MEDIAN
  int d2=GLBsensorHC.getLatestDistanceRead();
#endif


  if (d > DISTANCE_ZERO)            { 
#ifdef LAMP_DEBUG_STATES
    Serial.print(F("--->NEW STATE: STATE_idle ("));
    Serial.print(d);
#ifdef LAMP_DEBUG_HCSR04_MEDIAN
    Serial.print(F(") "));
    Serial.print(d2);
#endif
    Serial.println(".");
#endif
#ifdef LAMP_ENABLE_LSLIB
    GLBledStrip.processCommands(LSidle,true);  
    GLB_lsTempBK=0;
#endif
    GLBptrStateFunc=STATE_idle; 
    //tone(PIN_BUZZER,2000,50); 
    return;
  } 


  if (GLB_timerZ1 == -1){
    GLB_timerZ1=mainTimers.setTimeout(TIMEOUT_Z1,GLBcallbackTimeoutZ1);
  }

  if (GLB_timerZ1_expired){
    GLB_timerZ1_expired=false;

#ifdef LAMP_DEBUG_STATES
    Serial.print(F("--->NEW STATE: STATE_distanceZero_T1 ("));
    Serial.print(d);
#ifdef LAMP_DEBUG_HCSR04_MEDIAN
    Serial.print(F(") "));
    Serial.print(d2);
#endif
    Serial.println(".");
#endif

    resetTimerZ();
    GLB_timerZ1=mainTimers.setTimeout(TIMEOUT_Z2,GLBcallbackTimeoutZ1);
    GLBsingTemp();
#ifdef LAMP_ENABLE_LSLIB
    GLBledStrip.processCommands(LSdistanceZeroT1,true);  
#endif
    GLBptrStateFunc=STATE_distanceZero_T1; 
    return;
  }

  rtttl::stop(); //Just in case,silence pls
 

}

//-------------------------------------------------
void STATE_distanceZero_T1(void)
{
 
  int d=GLBsensorHC.getLatestDistanceMedian();
#ifdef LAMP_DEBUG_HCSR04_MEDIAN
  int d2=GLBsensorHC.getLatestDistanceRead();
#endif

  if (d > DISTANCE_ZERO)            { 
#ifdef LAMP_DEBUG_STATES
    Serial.print(F("--->NEW STATE: STATE_idle ("));
    Serial.print(d);
#ifdef LAMP_DEBUG_HCSR04_MEDIAN
    Serial.print(F(") "));
    Serial.print(d2);
#endif
    Serial.println(".");
#endif
    rtttl::stop();
#ifdef LAMP_ENABLE_LSLIB
    GLBledStrip.processCommands(LSidle,true);  
    GLB_lsTempBK=0;
#endif
    GLBptrStateFunc=STATE_idle; 
    return;
  } 

  if (GLB_timerZ1_expired){
    GLB_timerZ1_expired=false;
#ifdef LAMP_DEBUG_STATES
    Serial.print(F("--->NEW STATE: STATE_distanceZero_T2 ("));
    Serial.print(d);
#ifdef LAMP_DEBUG_HCSR04_MEDIAN
    Serial.print(F(") "));
    Serial.print(d2);
#endif
    Serial.println(".");
#endif
    GLBsingHum();
#ifdef LAMP_ENABLE_LSLIB
    GLBledStrip.processCommands(LSdistanceZeroT2,true);  
#endif
    GLBptrStateFunc=STATE_distanceZero_T2;
    return;
  }

}


//-------------------------------------------------
void STATE_distanceZero_T2(void)
{
 
  int d=GLBsensorHC.getLatestDistanceMedian();
#ifdef LAMP_DEBUG_HCSR04_MEDIAN
  int d2=GLBsensorHC.getLatestDistanceRead();
#endif

  if (d > DISTANCE_ZERO)            { 
#ifdef LAMP_DEBUG_STATES
    Serial.print(F("--->NEW STATE: STATE_idle ("));
    Serial.print(d);
#ifdef LAMP_DEBUG_HCSR04_MEDIAN
    Serial.print(F(") "));
    Serial.print(d2);
#endif
    Serial.println(".");
#endif

    rtttl::stop();
#ifdef LAMP_ENABLE_LSLIB
    GLBledStrip.processCommands(LSidle,true);  
    GLB_lsTempBK=0;
#endif
    GLBptrStateFunc=STATE_idle;     
    return;
  } 

  if (!rtttl::isPlaying()) {
#ifdef LAMP_DEBUG_STATES
    Serial.print(F("--->NEW STATE: STATE_distanceZero_idle ("));
    Serial.print(d);
#ifdef LAMP_DEBUG_HCSR04_MEDIAN
    Serial.print(F(") "));
    Serial.print(d2);
#endif
    Serial.println(".");
#endif

#ifdef LAMP_ENABLE_LSLIB
    GLBledStrip.processCommands(LSdistanceZeroIdle,true);  
#endif
    GLBptrStateFunc=STATE_distanceZero_idle;
    return;
  }

}

//-------------------------------------------------
void STATE_distanceZero_idle(void)
{
 
  int d=GLBsensorHC.getLatestDistanceMedian();
#ifdef LAMP_DEBUG_HCSR04_MEDIAN
  int d2=GLBsensorHC.getLatestDistanceRead();
#endif

  if (d > DISTANCE_ZERO)            { 
#ifdef LAMP_DEBUG_STATES
    Serial.print(F("--->NEW STATE: STATE_idle ("));
    Serial.print(d);
#ifdef LAMP_DEBUG_HCSR04_MEDIAN
    Serial.print(F(") "));
    Serial.print(d2);
#endif
    Serial.println(".");
#endif

#ifdef LAMP_ENABLE_LSLIB
    GLBledStrip.processCommands(LSidle,true);  
    GLB_lsTempBK=0;
#endif
    GLBptrStateFunc=STATE_idle; 
    return;
  } 


  rtttl::stop(); //Just in case,silence pls

}

//-------------------------------------------------
void STATE_distanceA(void)
{
  int d=GLBsensorHC.getLatestDistanceMedian();
#ifdef LAMP_DEBUG_HCSR04_MEDIAN
  int d2=GLBsensorHC.getLatestDistanceRead();
#endif  
  if (d < DISTANCE_ZERO)            { 
#ifdef LAMP_DEBUG_STATES
    Serial.print(F("--->NEW STATE: STATE_distanceZero ("));
    Serial.print(d);
#ifdef LAMP_DEBUG_HCSR04_MEDIAN
    Serial.print(F(") "));
    Serial.print(d2);
#endif
    Serial.println(".");
#endif

    rtttl::stop(); 
#ifdef LAMP_ENABLE_LSLIB
    GLBledStrip.processCommands(LSdistanceZero,true);  
#endif
    GLBptrStateFunc=STATE_distanceZero; 
    return;
  } 
  if (d < DISTANCE_A) {
    if (!rtttl::isPlaying()) {  //Sing another
      rtttl::beginF(PIN_BUZZER,audiosTracker.getOrderedItem(true));  
    }
    //Tune speed
    bool bk=singingQuickly;
    if (d < DISTANCE_A2) {singingQuickly=false;}
    else                 {singingQuickly=true;}

    if (bk!=singingQuickly){
      if (d < DISTANCE_A2) {
        rtttl::changeSpeed(1);     
#ifdef LAMP_ENABLE_LSLIB
        GLBledStrip.processCommands(LSdistanceA,true);  
#endif
      }
      else {
        rtttl::changeSpeed(2);     
#ifdef LAMP_ENABLE_LSLIB
        GLBledStrip.processCommands(LSdistanceAQuickly,true);  
#endif
      }
    }

    

    return;
  }


  //Here only enters if distance is above A!!, let's song finish in other state
  //It should be playing, just in case checking it....
  if (rtttl::isPlaying()) {
#ifdef LAMP_DEBUG_STATES
    Serial.print(F("--->NEW STATE: STATE_singing ("));
    Serial.print(d);
#ifdef LAMP_DEBUG_HCSR04_MEDIAN
    Serial.print(F(") "));
    Serial.print(d2);
#endif
    Serial.println(".");
#endif

#ifdef LAMP_ENABLE_LSLIB
    GLBledStrip.processCommands(LSsinging,true);  
#endif
    GLBptrStateFunc=STATE_singing;
    //tone(PIN_BUZZER,2000,50); 
    return;
  }

  //Bizarre, no playing an distance above A, go to idle
  
#ifdef LAMP_DEBUG_STATES
  Serial.print(F("--->NEW STATE: STATE_idle ("));
  Serial.print(d);
#ifdef LAMP_DEBUG_HCSR04_MEDIAN
  Serial.print(F(") "));
  Serial.print(d2);
#endif
  Serial.println(".");
#endif

}

//-------------------------------------------------
void STATE_singing(void)
{
  int d=GLBsensorHC.getLatestDistanceMedian();
#ifdef LAMP_DEBUG_HCSR04_MEDIAN
  int d2=GLBsensorHC.getLatestDistanceRead();
#endif
  if (d < DISTANCE_ZERO)            { 
#ifdef LAMP_DEBUG_STATES
    Serial.print(F("--->NEW STATE: STATE_distanceZero ("));
    Serial.print(d);
#ifdef LAMP_DEBUG_HCSR04_MEDIAN
    Serial.print(F(") "));
    Serial.print(d2);
#endif
    Serial.println(".");
#endif

    rtttl::stop(); 
#ifdef LAMP_ENABLE_LSLIB
    GLBledStrip.processCommands(LSdistanceZero,true);  
#endif
    GLBptrStateFunc=STATE_distanceZero; 
    return;
  } 
  if (d < DISTANCE_A) {
#ifdef LAMP_DEBUG_STATES
    Serial.print(F("--->NEW STATE: STATE_distanceA ("));
    Serial.print(d);
#ifdef LAMP_DEBUG_HCSR04_MEDIAN
    Serial.print(F(") "));
    Serial.print(d2);
#endif
    Serial.println(".");
#endif

    // Changing  the song !!!!!!!!!! 
    rtttl::beginF(PIN_BUZZER,audiosTracker.getOrderedItem(true)); 
#ifdef LAMP_ENABLE_LSLIB 
    GLBledStrip.processCommands(LSdistanceA,true);  
#endif
    GLBptrStateFunc=STATE_distanceA;
    return;
  }

  if (!rtttl::isPlaying()) {
#ifdef LAMP_DEBUG_STATES
    Serial.print(F("--->NEW STATE: STATE_idle ("));
    Serial.print(d);
#ifdef LAMP_DEBUG_HCSR04_MEDIAN
    Serial.print(F(") "));
    Serial.print(d2);
#endif
    Serial.println(".");
#endif

#ifdef LAMP_ENABLE_LSLIB
    GLBledStrip.processCommands(LSidle,true); 
    GLB_lsTempBK=0; 
#endif
    GLBptrStateFunc=STATE_idle;
    return;
  }
  


}
int m=0; //TODO remove
int n=0; //TODO remove
//-------------------------------------------------
//-------------------------------------------------
//-------------------------------------------------
void loop() { 
 
  //------------- INPUT REFRESHING ----------------
  GLBsensorDHT.refresh();
  GLBsensorHC.refresh();
    
  //--------- TIME TO THINK MY FRIEND -------------
  // State machine as main controller execution
  GLBptrStateFunc();
  mainTimers.run();

  // ------------- OUTPUT REFRESHING ---------------
#ifdef LAMP_ENABLE_LSLIB
  GLBledStrip.refresh();
  FastLED.show();
#endif
  //Serial.println(F("PLAY_GO"));
  rtttl::play();
  //Serial.println(F("PLAY_OUT"));

  /*TODO MOVE TO STATES */ 
  char m_str[3];
  u8g2.firstPage();
  do {
    u8g2.setFont(u8g2_font_logisoso32_tn);
    strcpy(m_str, u8x8_u8toa(n, 1));
    u8g2.drawStr(0,32,n);
    u8g2.drawStr(33,32,":");
    strcpy(m_str, u8x8_u8toa(m, 2));
    u8g2.drawStr(50,32,m_str);
  } while ( u8g2.nextPage() );
  delay(1000);
  m++;
  if ( m == 60 ){
    m = 0;
    n++;
  }


}




