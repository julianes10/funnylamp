#include <stdio.h>
#include "lsem.h"
#include "LowPower.h"
#include "SimpleTimer.h"
#include "DHTsensor.h"

#include "HCSR04sensor.h"
#include "NonBlockingRtttl.h"
#include "RtttlTrackerList.h"


//------------------------------------------------
//--- DEBUG  FLAGS     ---------------------------
//------------------------------------------------
//#define LAMP_HCSR04_MEDIAN_DEBUG
//#define LAMP_STATES_DEBUG


//------------------------------------------------
//--- ENABLE  FLAGS     --------------------------
//------------------------------------------------
//#define ENABLE_SERIAL_INPUT


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

#define MEDIAN_FILTER_SIZE_SENSOR_HCSR04  5

// Sensors
DHTsensor    GLBsensorDHT;
HCSR04sensor GLBsensorHC(PIN_SENSOR_HCSR04_TRIGGER,PIN_SENSOR_HCSR04_ECHO,INT_SENSOR_HCSR04,MEDIAN_FILTER_SIZE_SENSOR_HCSR04);

// Misc 
unsigned int aliveLoopCounter=0;
unsigned int msCounter=0;
unsigned int sCounter=0;
unsigned int latestTempReported=0;


char    GLBauxString[100];  //for copying PROGMEM to RAM,use it in rtt and ls to save ram and avoid corruption issues.

#ifdef ENABLE_SERIAL_INPUT
char    GLBserialInputString[100]; // a string to hold incoming data
int     GLBserialIx=0;
bool    GLBserialInputStringReady = false; // whether the string is complete
#endif

const char inputBootString[] PROGMEM ={":LP0020:LT0005:LMK:LCFF,00,00:LQ:LMk:LQ:LMN"};
const char inputTest[]  PROGMEM = {":LP0200:LT0050:LMN:LQ:LMA:LP0100:LCFF,00,00:LQ:LMA:LP0100:LC00,FF,00:LQ:LMA:LP0100:LC00,00,FF"};

const char LSdistanceZero[]  PROGMEM = {":LMN:LP0000"};
const char LSdistanceA[]  PROGMEM = {":LX:LMK:LP0000:LT0010:LCFF,00,00"};
const char LSdistanceB[]  PROGMEM = {":LX:LMK:LP0000:LT0010,LC00,FF,00"};
const char LSdistanceC[]  PROGMEM = {":LX:LMK:LP0000:LT0010,LC00,00,FF"};
const char LSoff[]  PROGMEM = {":LX"};

const char LStempA[]  PROGMEM = {":LX:LMA:LP0000:LT0000:LC50,50,FF"};
const char LStempB[]  PROGMEM = {":LX:LMA:LP0000:LT0000:LC46,82,82"};
const char LStempC[]  PROGMEM = {":LX:LMA:LP0000:LT0000:LC73,D1,8B"};
const char LStempD[]  PROGMEM = {":LX:LMA:LP0000:LT0000:LCEE,F7,70"};
const char LStempE[]  PROGMEM = {":LX:LMA:LP0000:LT0000:LCF2,AD,35"};
const char LStempF[]  PROGMEM = {":LX:LMA:LP0000:LT0000:LCFF,50,20"};



#define NUM_LEDS 3
CRGB GLBledStripBufferMem[NUM_LEDS];

//--------- Global functions declarations ------------
void(* resetFunc) (void) = 0;//declare reset function at address 0
void GLBcallbackTimeoutLS(void);
void GLBcallbackPauseLS(void);

LSEM GLBledStrip(GLBledStripBufferMem,NUM_LEDS,GLBcallbackPauseLS,GLBcallbackTimeoutLS);

unsigned long time;
SimpleTimer mainTimers;

//--------------- SONGS ------------------------
const char  mario[] PROGMEM = {"mario:d=4,o=5,b=100:16e6,16e6,32p,8e6,16c6,8e6,8g6,8p,8g,8p,8c6,16p,8g,16p,8e,16p,8a,8b,16a#,8a,16g.,16e6,16g6,8a6,16f6,8g6,8e6,16c6,16d6,8b,16p,8c6,16p,8g,16p,8e,16p,8a,8b,16a#,8a,16g.,16e6,16g6,8a6,16f6,8g6,8e6,16c6,16d6,8b,8p,16g6,16f#6,16f6,16d#6,16p,16e6,16p,16g#,16a,16c6,16p,16a,16c6,16d6,8p,16g6,16f#6,16f6,16d#6,16p,16e6,16p,16c7,16p,16c7,16c7,p,16g6,16f#6,16f6,16d#6,16p,16e6,16p,16g#,16a,16c6,16p,16a,16c6,16d6,8p,16d#6,8p,16d6,8p,16c6\0"};

const char  mi[] PROGMEM = {"MissionImp:d=16,o=6,b=95:32d,32d#,32d,32d#,32d,32d#,32d,32d#,32d,32d,32d#,32e,32f,32f#,32g,g,8p,g,8p,a#,p,c7,p,g,8p,g,8p,f,p,f#,p,g,8p,g,8p,a#,p,c7,p,g,8p,g,8p,f,p,f#,p,a#,g,2d,32p,a#,g,2c#,32p,a#,g,2c,a#5,8c,2p,32p,a#5,g5,2f#,32p,a#5,g5,2f,32p,a#5,g5,2e,d#,8d\0"};

const char  indiana[] PROGMEM = {"Indiana:d=4,o=5,b=250:e,8p,8f,8g,8p,1c6,8p.,d,8p,8e,1f,p.,g,8p,8a,8b,8p,1f6,p,a,8p,8b,2c6,2d6,2e6,e,8p,8f,8g,8p,1c6,p,d6,8p,8e6,1f.6,g,8p,8g,e.6,8p,d6,8p,8g,e.6,8p,d6,8p,8g,f.6,8p,e6,8p,8d6,2c6\0"};


const char  StarWars[] PROGMEM = {"StarWars:d=4,o=5,b=45:32p,32f#,32f#,32f#,8b.,8f#.6,32e6,32d#6,32c#6,8b.6,16f#.6,32e6,32d#6,32c#6,8b.6,16f#.6,32e6,32d#6,32e6,8c#.6,32f#,32f#,32f#,8b.,8f#.6,32e6,32d#6,32c#6,8b.6,16f#.6,32e6,32d#6,32c#6,8b.6,16f#.6,32e6,32d#6,32e6,8c#6\0"};

const char  AbbaMammaMia[] PROGMEM = {"AbbaMammaMia:d=4,o=5,b=40:32f6,32d#6,32f6,32d#6,8p,32d#6,32d#6,32f6,32g6,32f6,32d#6,16p,16f6,32d#6,16p,16g#6,32g#6,32g#6,32g#6,16g6,16d#6,16p,32f6,32d#6,32f6,32d#6,8p,32d#6,32d#6,32f6,32g6,32f6,32d#6,16p,16f6,32d#6,16p,16g#6,32g#6,32g#6,32g#6,16g6,16d#6,16p,16a#.6,32a#6,32a#6,16a#6,16f6,16g6,16g#6,16p,32p,16g.6,32g6,32g6,16g6,16d6,16d#6,16f6,16p,32p,16f6,16d#6,32p,16g#6,32g#6,32g#6,32g#6,32g6,32d#6,32f6,32d#6\0"};


const char  Imperial[] PROGMEM = {"Imperial:d=4,o=5,b=100:e,e,e,8c,16p,16g,e,8c,16p,16g,e,p,b,b,b,8c6,16p,16g,d#,8c,16p,16g,e,8p\0"};



const char  Supercalif[] PROGMEM = {"Supercalif:d=32,o=5,b=45:f,g#,g#,g#,a#,g#,g#,f,g#,g#,a#,g#,16g#,16f#,g#,g#,g#,g#,a#,g#,g#,d#,g#,g#,a#,g#,16g#,16f,g#,g#,g#,g#,a#,g#,g#,g#,c#6,c#6,d#6,c#6,16c#6,16a#,a#,c#6,c6,a#,c#6,g#,g#,f,g#,a,a#,c6,16c#6,16c#6\0"};



const char  Twinkle[] PROGMEM = {"Twinkle:d=4,o=5,b=80:32p,8c,8c,8g,8g,8a,8a,g,8f,8f,8e,8e,8d,8d,c,8g,8g,8f,8f,8e,8e,d,8g,8g,8f,8f,8e,8e,d,8c,8c,8g,8g,8a,8a,g\0"};

const char warmingUp[] PROGMEM ={"TwinkleShort:d=4,o=5,b=80:32p,8c,8c,8g,8g,8a,8a,g\0"};



#define MAX_AUDIOS 8
/* ALTERNATIVE WITHOUT TRACKING 
const char *audiosTable[MAX_AUDIOS]= {
  mario,mi,indiana
};
*/

RtttlTrackerItem  audios[MAX_AUDIOS];
RtttlTrackerList  audiosTracker;


#define DISTANCE_ZERO   10
#define DISTANCE_A      40


//------------------------------------------------
//--- GLOBAL FUNCTIONS ---------------------------
//------------------------------------------------


//-- SOME NEEDED PROTOTYPES ---------------------------

void STATE_init(void);
void STATE_welcome(void);
void STATE_idle(void);
#ifdef ENABLE_SERIAL_INPUT
void STATE_LDcmd(void);
#endif
void STATE_distanceZero(void);
void STATE_distanceA(void);
void STATE_singing(void);

// State pointer function
void (*GLBptrStateFunc)();


//------------------------------------------------
void sendFlashStringLS(const char *str)
{
  strcpy_P(GLBauxString,(char*)str);
  GLBledStrip.processCommands(GLBauxString);
}


//------------------------------------------------
//-------    TIMER CALLBACKS     -----------------
//------------------------------------------------
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
void GLBcallbackLoggingLedStrip(void)
{
  Serial.println(F("DEBUG: Led strip..."));
  sendFlashStringLS(inputTest);
}
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
void GLBsingTemp()
{
  float t=GLBsensorDHT.getTemperature();
  float h=GLBsensorDHT.getHumidity();

  int dec = (int) t/10;
  int units= (t - dec*10);
  int i=0;



  for (i=0;i<dec;i++){
    tone(PIN_BUZZER,1000,20);
    delay(100);
  }

  delay(500);
  for (i=0;i<units;i++){
    tone(PIN_BUZZER,2000,20);
    delay(100);
  } 

}
//------------------------------------------------
//------------------------------------------------
//------------------------------------------------

void setup() { 
  // Serial to debug AND comunication protocolo with PI              
  Serial.begin(9600);
  Serial.println(F("Setup... 'came on,be my baby,came on'"));

#ifdef ENABLE_SERIAL_INPUT
  GLBserialInputString[0]=0;
#endif 
  GLBptrStateFunc = STATE_init;
  Serial.println(F("STATE INIT"));
  FastLED.addLeds<WS2812B,PIN_LED_STRIP,GRB>(GLBledStripBufferMem,NUM_LEDS);

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



  randomSeed(analogRead(0));

  //memset(audios,0,sizeof(RtttlTrackerItem)*MAX_AUDIOS);
  audiosTracker.setup(audios,MAX_AUDIOS);
  audiosTracker.addItem(mario);
  audiosTracker.addItem(mi);
  audiosTracker.addItem(indiana);
  audiosTracker.addItem(StarWars);
  audiosTracker.addItem(AbbaMammaMia);
  audiosTracker.addItem(Imperial);
  audiosTracker.addItem(Supercalif);
  audiosTracker.addItem(Twinkle);





  //REMINDER IF ADD MORE,MAKE SURE MAX_AUDIOS is sync

}

//------------------------------------------------

#ifdef ENABLE_SERIAL_INPUT

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
#ifdef ENABLE_SERIAL_INPUT
void processSerialInputString()
{
  strcpy(GLBauxString,GLBserialInputString);
  GLBserialInputString[0]=0;
  GLBserialInputStringReady = false;
  GLBledStrip.processCommands(GLBauxString);
}
#endif

//-------------------------------------------------
void STATE_init(void)
{
  #ifdef LAMP_STATES_DEBUG
  Serial.println(F("STATE INIT -> WELCOME"));
  #endif
  tone(PIN_BUZZER,1000,20);

  digitalWrite(PIN_LED_WHITE,HIGH);
  delay(500);
  digitalWrite(PIN_LED_WHITE,LOW);
  delay(500);
  digitalWrite(PIN_LED_WHITE,HIGH);

  sendFlashStringLS(inputBootString);
  GLBptrStateFunc=STATE_welcome;
  rtttl::beginF(PIN_BUZZER,warmingUp);

}
//-------------------------------------------------
void STATE_welcome(void)
{
  if (GLBledStrip.isIdle() && !rtttl::isPlaying()) {
    //Force a pause TODO ADD A NEW STAGE AND NON-BLOCKING SING TEMP
    delay(1000);
    GLBsingTemp();
    GLBptrStateFunc=STATE_idle;
#ifdef LAMP_STATES_DEBUG
    Serial.println(F("STATE WELCOME -> IDLE"));
#endif
  }
}
//-------------------------------------------------
void STATE_idle(void)
{
  int d=GLBsensorHC.getLatestDistanceMedian();
#ifdef LAMP_HCSR04_MEDIAN_DEBUG
  int d2=GLBsensorHC.getLatestDistanceRead();
#endif

#ifdef ENABLE_SERIAL_INPUT
  if (GLBserialInputStringReady){
    processSerialInputString();
    GLBptrStateFunc=STATE_LDcmd;
#ifdef LAMP_STATES_DEBUG
    Serial.println(F("STATE IDLE -> CMD LS"));
#endif 
  }
#endif

  if (d < DISTANCE_ZERO)            { 
#ifdef LAMP_STATES_DEBUG
    Serial.print(F("--->NEW STATE: STATE_distanceZero ("));
    Serial.print(d);
#ifdef LAMP_HCSR04_MEDIAN_DEBUG
    Serial.print(F(") "));
    Serial.println(d2);
#endif
#endif
    GLBptrStateFunc=STATE_distanceZero; 
    sendFlashStringLS(LSdistanceZero);  
    rtttl::stop(); 
  }  
  if (d > DISTANCE_ZERO   && d< DISTANCE_A)  { 
#ifdef LAMP_STATES_DEBUG
    Serial.print(F("--->NEW STATE: STATE_distanceA ("));
    Serial.print(d);
#ifdef LAMP_HCSR04_MEDIAN_DEBUG
    Serial.print(F(") "));
    Serial.println(d2);
#endif
#endif
    GLBptrStateFunc=STATE_distanceA;
    sendFlashStringLS(LSdistanceA);  
//    rtttl::beginF(PIN_BUZZER,audiosTracker.getRandomItem(false,true));
    rtttl::beginF(PIN_BUZZER,audiosTracker.getOrderedItem(true));  

  }

  if (GLBptrStateFunc!=STATE_idle) return;
 
  float t=GLBsensorDHT.getTemperature();
  if      (t < 15.0)  { sendFlashStringLS(LStempA);} 
  else if (t < 21.0)  { sendFlashStringLS(LStempB);}  
  else if (t < 25.0)  { sendFlashStringLS(LStempC);} 
  else if (t < 28.0)  { sendFlashStringLS(LStempD);} 
  else if (t < 31.0)  { sendFlashStringLS(LStempE);} 
  else                { sendFlashStringLS(LStempF);} 
}


#ifdef ENABLE_SERIAL_INPUT
//-------------------------------------------------
void STATE_LDcmd(void)
{
  if (GLBserialInputStringReady){
    processSerialInputString();
  }
  else if (GLBledStrip.isIdle()) {
    GLBptrStateFunc=STATE_idle;
#ifdef LAMP_STATES_DEBUG
    Serial.println(F("STATE LD CMD -> IDLE"));
#endif
  }
}
#endif

//-------------------------------------------------
void STATE_distanceZero(void)
{
  int d=GLBsensorHC.getLatestDistanceMedian();
#ifdef LAMP_HCSR04_MEDIAN_DEBUG
  int d2=GLBsensorHC.getLatestDistanceRead();
#endif
/*

    Serial.print(F(" ("));
    Serial.print(d);
#ifdef LAMP_HCSR04_MEDIAN_DEBUG
    Serial.print(F(") "));
    Serial.println(d2);
#endif    Serial.print(F(") "));
*/

  if (d > DISTANCE_ZERO)            { 
#ifdef LAMP_STATES_DEBUG
    Serial.print(F("--->NEW STATE: STATE_idle ("));
    Serial.print(d);
#ifdef LAMP_HCSR04_MEDIAN_DEBUG
    Serial.print(F(") "));
    Serial.println(d2);
#endif
#endif
    GLBptrStateFunc=STATE_idle; 
    //tone(PIN_BUZZER,2000,50); 
    return;
  } 
  rtttl::stop(); //Just in case,silence pls
}

//-------------------------------------------------
void STATE_distanceA(void)
{
  int d=GLBsensorHC.getLatestDistanceMedian();
#ifdef LAMP_HCSR04_MEDIAN_DEBUG
  int d2=GLBsensorHC.getLatestDistanceRead();
#endif  
  if (d < DISTANCE_ZERO)            { 
#ifdef LAMP_STATES_DEBUG
    Serial.print(F("--->NEW STATE: STATE_distanceZero ("));
    Serial.print(d);
#ifdef LAMP_HCSR04_MEDIAN_DEBUG
    Serial.print(F(") "));
    Serial.println(d2);
#endif
#endif
    GLBptrStateFunc=STATE_distanceZero; 
    sendFlashStringLS(LSdistanceZero);  
    rtttl::stop(); 
    //tone(PIN_BUZZER,2000,50);  
    return;
  } 
  if (d < DISTANCE_A) {
    //keep it here! TODO sing other song if still here.
    return;
  }

  if (rtttl::isPlaying()) {
#ifdef LAMP_STATES_DEBUG
    Serial.print(F("--->NEW STATE: STATE_singing ("));
    Serial.print(d);
#ifdef LAMP_HCSR04_MEDIAN_DEBUG
    Serial.print(F(") "));
    Serial.println(d2);
#endif
#endif
    GLBptrStateFunc=STATE_singing;
    //tone(PIN_BUZZER,2000,50); 
    return;
  }
  else {
#ifdef LAMP_STATES_DEBUG
    Serial.print(F("--->NEW STATE: STATE_idle ("));
    Serial.print(d);
#ifdef LAMP_HCSR04_MEDIAN_DEBUG
    Serial.print(F(") "));
    Serial.println(d2);
#endif
#endif
    //tone(PIN_BUZZER,2000,50);  
  }
}

//-------------------------------------------------
void STATE_singing(void)
{
  int d=GLBsensorHC.getLatestDistanceMedian();
#ifdef LAMP_HCSR04_MEDIAN_DEBUG
  int d2=GLBsensorHC.getLatestDistanceRead();
#endif
  if (d < DISTANCE_ZERO)            { 
#ifdef LAMP_STATES_DEBUG
    Serial.print(F("--->NEW STATE: STATE_distanceZero ("));
    Serial.print(d);
#ifdef LAMP_HCSR04_MEDIAN_DEBUG
    Serial.print(F(") "));
    Serial.println(d2);
#endif
#endif
    GLBptrStateFunc=STATE_distanceZero; 
    sendFlashStringLS(LSdistanceZero);  
    rtttl::stop(); 
    //tone(PIN_BUZZER,2000,50); 
    return;
  } 
  if (d < DISTANCE_A) {
#ifdef LAMP_STATES_DEBUG
    Serial.print(F("--->NEW STATE: STATE_distanceA ("));
    Serial.print(d);
#ifdef LAMP_HCSR04_MEDIAN_DEBUG
    Serial.print(F(") "));
    Serial.println(d2);
#endif
#endif
    GLBptrStateFunc=STATE_distanceA;
    sendFlashStringLS(LSdistanceA);  
    //tone(PIN_BUZZER,2000,20); 
    //delay(20);
    //tone(PIN_BUZZER,2000,20); 
    //delay(20);
//    rtttl::beginF(PIN_BUZZER,audiosTracker.getRandomItem(false,true));
    rtttl::beginF(PIN_BUZZER,audiosTracker.getOrderedItem(true));  

    return;
  }

  if (!rtttl::isPlaying()) {
#ifdef LAMP_STATES_DEBUG
    Serial.print(F("--->NEW STATE: STATE_idle ("));
    Serial.print(d);
#ifdef LAMP_HCSR04_MEDIAN_DEBUG
    Serial.print(F(") "));
    Serial.println(d2);
#endif
#endif
    GLBptrStateFunc=STATE_idle;
    return;
  }
}

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
  GLBledStrip.refresh();
  FastLED.show();

  //Serial.println(F("PLAY_GO"));
  rtttl::play();
  //Serial.println(F("PLAY_OUT"));


}




