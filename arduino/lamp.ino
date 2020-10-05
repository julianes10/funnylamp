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

#define MEDIAN_FILTER_SIZE_SENSOR_HCSR04  15

// Sensors
DHTsensor    GLBsensorDHT;
HCSR04sensor GLBsensorHC(PIN_SENSOR_HCSR04_TRIGGER,PIN_SENSOR_HCSR04_ECHO,INT_SENSOR_HCSR04,MEDIAN_FILTER_SIZE_SENSOR_HCSR04);

// Misc 
unsigned int aliveLoopCounter=0;
unsigned int msCounter=0;
unsigned int sCounter=0;
unsigned int latestTempReported=0;

bool singingQuickly=false;



#ifdef ENABLE_SERIAL_INPUT
char    GLBauxString[100];  
char    GLBserialInputString[100]; // a string to hold incoming data
int     GLBserialIx=0;
bool    GLBserialInputStringReady = false; // whether the string is complete
#endif

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
const char LStempWarm[]       PROGMEM = {":LX:LMA:LT0000:LP0000:LCFB,F7,F1\0"};
const char LStempVeryWarm[]   PROGMEM = {":LX:LMA:LT0000:LP0000:LCFB,D9,71\0"};;
const char LStempHot[]        PROGMEM = {":LX:LMA:LT0000:LP0000:LCFB,A0,22\0"};
const char LStempVeryHot[]    PROGMEM = {":LX:LMA:LT0000:LP0000:LCFF,20,20\0"};


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


#define DISTANCE_ZERO    12
#define DISTANCE_A       40
#define DISTANCE_A2      27


#define TIMEOUT_Z1  3500
#define TIMEOUT_Z2  6500  // it covers singing temp
int GLB_timerZ1=-1;
bool GLB_timerZ1_expired=false;




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
  GLBledStrip.processCommands(inputTest,true);
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
void GLBcallbackTimeoutZ1(void)
{
  if (GLB_timerZ1 != -1)     
    mainTimers.deleteTimer(GLB_timerZ1);
  GLB_timerZ1=-1;
  GLB_timerZ1_expired=true;
}


//------------------------------------------------
void resetTimerZ(void)
{
  GLBcallbackTimeoutZ1();
  GLB_timerZ1_expired=false;
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
  GLBledStrip.processCommands(GLBauxString,true);
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

  GLBledStrip.processCommands(LSwelcome,true);
  GLBptrStateFunc=STATE_welcome;
  rtttl::beginF(PIN_BUZZER,warmingUp);

}
//-------------------------------------------------
void STATE_welcome(void)
{
  if (GLBledStrip.isIdle() && !rtttl::isPlaying()) {
    //Force a pause TODO ADD A NEW STAGE AND NON-BLOCKING SING TEMP
    delay(1000);
    GLBledStrip.processCommands(LSidle,true);  
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

  resetTimerZ();


  if (d < DISTANCE_ZERO)            { 
#ifdef LAMP_STATES_DEBUG
    Serial.print(F("--->NEW STATE: STATE_distanceZero ("));
    Serial.print(d);
#ifdef LAMP_HCSR04_MEDIAN_DEBUG
    Serial.print(F(") "));
    Serial.print(d2);
#endif
    Serial.println(".");
#endif

    rtttl::stop(); 
    GLBledStrip.processCommands(LSdistanceZero,true);  
    GLBptrStateFunc=STATE_distanceZero; 
  }  
  if (d > DISTANCE_ZERO   && d < DISTANCE_A)  { 
#ifdef LAMP_STATES_DEBUG
    Serial.print(F("--->NEW STATE: STATE_distanceA ("));
    Serial.print(d);
#ifdef LAMP_HCSR04_MEDIAN_DEBUG
    Serial.print(F(") "));
    Serial.print(d2);
#endif
    Serial.println(".");
#endif

    rtttl::beginF(PIN_BUZZER,audiosTracker.getOrderedItem(true));  
    GLBledStrip.processCommands(LSdistanceA,true); 
    GLBptrStateFunc=STATE_distanceA;
  }

  if (GLBptrStateFunc!=STATE_idle) return;
 
  float t=GLBsensorDHT.getTemperature();
  if      (t < 15.0)  { GLBledStrip.processCommands(LStempVeryCold,true);} 
  else if (t < 18.0)  { GLBledStrip.processCommands(LStempCold,true);}  
  else if (t < 21.0)  { GLBledStrip.processCommands(LStempOK,true);} 
  else if (t < 24.0)  { GLBledStrip.processCommands(LStempWarm,true);} 
  else if (t < 27.0)  { GLBledStrip.processCommands(LStempVeryWarm,true);}
  else if (t < 30.0)  { GLBledStrip.processCommands(LStempHot,true);} 
  else                { GLBledStrip.processCommands(LStempVeryHot,true);} 
}


#ifdef ENABLE_SERIAL_INPUT
//-------------------------------------------------
void STATE_LDcmd(void)
{
  if (GLBserialInputStringReady){
    processSerialInputString();
  }
  else if (GLBledStrip.isIdle()) {
    GLBledStrip.processCommands(LSidle,true);  
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


  if (d > DISTANCE_ZERO)            { 
#ifdef LAMP_STATES_DEBUG
    Serial.print(F("--->NEW STATE: STATE_idle ("));
    Serial.print(d);
#ifdef LAMP_HCSR04_MEDIAN_DEBUG
    Serial.print(F(") "));
    Serial.print(d2);
#endif
    Serial.println(".");
#endif

    GLBledStrip.processCommands(LSidle,true);  
    GLBptrStateFunc=STATE_idle; 
    //tone(PIN_BUZZER,2000,50); 
    return;
  } 


  if (GLB_timerZ1 == -1){
    GLB_timerZ1=mainTimers.setTimeout(TIMEOUT_Z1,GLBcallbackTimeoutZ1);
  }

  if (GLB_timerZ1_expired){
    GLB_timerZ1_expired=false;

#ifdef LAMP_STATES_DEBUG
    Serial.print(F("--->NEW STATE: STATE_distanceZero_T1 ("));
    Serial.print(d);
#ifdef LAMP_HCSR04_MEDIAN_DEBUG
    Serial.print(F(") "));
    Serial.print(d2);
#endif
    Serial.println(".");
#endif

    resetTimerZ();
    GLB_timerZ1=mainTimers.setTimeout(TIMEOUT_Z2,GLBcallbackTimeoutZ1);
    GLBsingTemp();
    GLBledStrip.processCommands(LSdistanceZeroT1,true);  
    GLBptrStateFunc=STATE_distanceZero_T1; 
    return;
  }

  rtttl::stop(); //Just in case,silence pls
 

}

//-------------------------------------------------
void STATE_distanceZero_T1(void)
{
 
  int d=GLBsensorHC.getLatestDistanceMedian();
#ifdef LAMP_HCSR04_MEDIAN_DEBUG
  int d2=GLBsensorHC.getLatestDistanceRead();
#endif

  if (d > DISTANCE_ZERO)            { 
#ifdef LAMP_STATES_DEBUG
    Serial.print(F("--->NEW STATE: STATE_idle ("));
    Serial.print(d);
#ifdef LAMP_HCSR04_MEDIAN_DEBUG
    Serial.print(F(") "));
    Serial.print(d2);
#endif
    Serial.println(".");
#endif
    rtttl::stop();
    GLBledStrip.processCommands(LSidle,true);  
    GLBptrStateFunc=STATE_idle; 
    return;
  } 

  if (GLB_timerZ1_expired){
    GLB_timerZ1_expired=false;
#ifdef LAMP_STATES_DEBUG
    Serial.print(F("--->NEW STATE: STATE_distanceZero_T2 ("));
    Serial.print(d);
#ifdef LAMP_HCSR04_MEDIAN_DEBUG
    Serial.print(F(") "));
    Serial.print(d2);
#endif
    Serial.println(".");
#endif
    GLBsingHum();
    GLBledStrip.processCommands(LSdistanceZeroT2,true);  
    GLBptrStateFunc=STATE_distanceZero_T2;
    return;
  }

}


//-------------------------------------------------
void STATE_distanceZero_T2(void)
{
 
  int d=GLBsensorHC.getLatestDistanceMedian();
#ifdef LAMP_HCSR04_MEDIAN_DEBUG
  int d2=GLBsensorHC.getLatestDistanceRead();
#endif

  if (d > DISTANCE_ZERO)            { 
#ifdef LAMP_STATES_DEBUG
    Serial.print(F("--->NEW STATE: STATE_idle ("));
    Serial.print(d);
#ifdef LAMP_HCSR04_MEDIAN_DEBUG
    Serial.print(F(") "));
    Serial.print(d2);
#endif
    Serial.println(".");
#endif

    rtttl::stop();
    GLBledStrip.processCommands(LSidle,true);  
    GLBptrStateFunc=STATE_idle;     
    return;
  } 

  if (!rtttl::isPlaying()) {
#ifdef LAMP_STATES_DEBUG
    Serial.print(F("--->NEW STATE: STATE_distanceZero_idle ("));
    Serial.print(d);
#ifdef LAMP_HCSR04_MEDIAN_DEBUG
    Serial.print(F(") "));
    Serial.print(d2);
#endif
    Serial.println(".");
#endif

    GLBledStrip.processCommands(LSdistanceZeroIdle,true);  
    GLBptrStateFunc=STATE_distanceZero_idle;
    return;
  }

}

//-------------------------------------------------
void STATE_distanceZero_idle(void)
{
 
  int d=GLBsensorHC.getLatestDistanceMedian();
#ifdef LAMP_HCSR04_MEDIAN_DEBUG
  int d2=GLBsensorHC.getLatestDistanceRead();
#endif

  if (d > DISTANCE_ZERO)            { 
#ifdef LAMP_STATES_DEBUG
    Serial.print(F("--->NEW STATE: STATE_idle ("));
    Serial.print(d);
#ifdef LAMP_HCSR04_MEDIAN_DEBUG
    Serial.print(F(") "));
    Serial.print(d2);
#endif
    Serial.println(".");
#endif

    GLBledStrip.processCommands(LSidle,true);  
    GLBptrStateFunc=STATE_idle; 
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
    Serial.print(d2);
#endif
    Serial.println(".");
#endif

    rtttl::stop(); 
    GLBledStrip.processCommands(LSdistanceZero,true);  
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
        GLBledStrip.processCommands(LSdistanceA,true);  
      }
      else {
        rtttl::changeSpeed(2);     
        GLBledStrip.processCommands(LSdistanceAQuickly,true);  
      }
    }

    

    return;
  }


  //Here only enters if distance is above A!!, let's song finish in other state
  //It should be playing, just in case checking it....
  if (rtttl::isPlaying()) {
#ifdef LAMP_STATES_DEBUG
    Serial.print(F("--->NEW STATE: STATE_singing ("));
    Serial.print(d);
#ifdef LAMP_HCSR04_MEDIAN_DEBUG
    Serial.print(F(") "));
    Serial.print(d2);
#endif
    Serial.println(".");
#endif

    GLBledStrip.processCommands(LSsinging,true);  
    GLBptrStateFunc=STATE_singing;
    //tone(PIN_BUZZER,2000,50); 
    return;
  }

  //Bizarre, no playing an distance above A, go to idle
  
#ifdef LAMP_STATES_DEBUG
  Serial.print(F("--->NEW STATE: STATE_idle ("));
  Serial.print(d);
#ifdef LAMP_HCSR04_MEDIAN_DEBUG
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
#ifdef LAMP_HCSR04_MEDIAN_DEBUG
  int d2=GLBsensorHC.getLatestDistanceRead();
#endif
  if (d < DISTANCE_ZERO)            { 
#ifdef LAMP_STATES_DEBUG
    Serial.print(F("--->NEW STATE: STATE_distanceZero ("));
    Serial.print(d);
#ifdef LAMP_HCSR04_MEDIAN_DEBUG
    Serial.print(F(") "));
    Serial.print(d2);
#endif
    Serial.println(".");
#endif

    rtttl::stop(); 
    GLBledStrip.processCommands(LSdistanceZero,true);  
    GLBptrStateFunc=STATE_distanceZero; 
    return;
  } 
  if (d < DISTANCE_A) {
#ifdef LAMP_STATES_DEBUG
    Serial.print(F("--->NEW STATE: STATE_distanceA ("));
    Serial.print(d);
#ifdef LAMP_HCSR04_MEDIAN_DEBUG
    Serial.print(F(") "));
    Serial.print(d2);
#endif
    Serial.println(".");
#endif

    // Changing  the song !!!!!!!!!! 
    rtttl::beginF(PIN_BUZZER,audiosTracker.getOrderedItem(true));  
    GLBledStrip.processCommands(LSdistanceA,true);  
    GLBptrStateFunc=STATE_distanceA;
    return;
  }

  if (!rtttl::isPlaying()) {
#ifdef LAMP_STATES_DEBUG
    Serial.print(F("--->NEW STATE: STATE_idle ("));
    Serial.print(d);
#ifdef LAMP_HCSR04_MEDIAN_DEBUG
    Serial.print(F(") "));
    Serial.print(d2);
#endif
    Serial.println(".");
#endif

    GLBledStrip.processCommands(LSidle,true);  
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




