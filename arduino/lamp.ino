#include <stdio.h>
#include "lsem.h"
#include "LowPower.h"
#include "SimpleTimer.h"
#include "DHTsensor.h"

#include "HCSR04sensor.h"




//------------------------------------------------
//--- GLOBAL VARIABLES ---------------------------
//------------------------------------------------


// PINS 

const int PIN_SENSOR_HCSR04_TRIGGER =3; 
const int PIN_SENSOR_HCSR04_ECHO    =2;  //INT NEEDED
const int PIN_LED_STRIP             =4;
const int PIN_LED_WHITE             =5; 
const int PIN_SENSOR_TEMP           =6;
const int PIN_BUZZER                =7;

// INTERRUPTS
const int INT_SENSOR_HCSR04         =0;

// Sensors
DHTsensor    GLBsensorDHT;
HCSR04sensor GLBsensorHC(PIN_SENSOR_HCSR04_TRIGGER, PIN_SENSOR_HCSR04_ECHO, INT_SENSOR_HCSR04);

// Misc 
unsigned int aliveLoopCounter=0;
unsigned int msCounter=0;
unsigned int sCounter=0;
unsigned int latestTempReported=0;


char    GLBserialInputString[100]; // a string to hold incoming data
char    GLBauxString[100];
int     GLBserialIx=0;
bool    GLBserialInputStringReady = false; // whether the string is complete

const char inputBootString[] PROGMEM ={":LP0020:LT0005:LMK:LCFF,00,00:LQ:LMk:LQ:LMN"};
const char inputTest[]  PROGMEM = {":LP0200:LT0050:LMN:LQ:LMA:LP0100:LCFF,00,00:LQ:LMA:LP0100:LC00,FF,00:LQ:LMA:LP0100:LC00,00,FF"};


#define NUM_LEDS 3
CRGB GLBledStripBufferMem[NUM_LEDS];

//--------- Global functions declarations ------------
void(* resetFunc) (void) = 0;//declare reset function at address 0
void GLBcallbackTimeoutLS(void);
void GLBcallbackPauseLS(void);

LSEM GLBledStrip(GLBledStripBufferMem, NUM_LEDS, GLBcallbackPauseLS, GLBcallbackTimeoutLS);

unsigned long time;
SimpleTimer mainTimers;

//------------------------------------------------
//--- GLOBAL FUNCTIONS ---------------------------
//------------------------------------------------


//-- SOME NEEDED PROTOTYPES ---------------------------

void STATE_init(void);
void STATE_welcome(void);
void STATE_idle(void);
void STATE_LDcmd(void);

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
  digitalWrite(PIN_LED_WHITE, !digitalRead(PIN_LED_WHITE));
}
//------------------------------------------------
void GLBcallbackLoggingLedStrip(void)
{
  Serial.println(F("DEBUG: Led strip..."));
  strcpy_P(GLBauxString,(char*)inputTest);
  GLBledStrip.processCommands(GLBauxString);
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


//------------------------------------------------

void setup() { 

  tone(PIN_BUZZER, 1000, 100);
  delay(100);
  tone(PIN_BUZZER, 1000, 100);
  // Serial to debug AND comunication protocolo with PI              
  Serial.begin(9600);
  Serial.println(F("Setup... 'came on, be my baby, came on'"));

  GLBserialInputString[0]=0;

  GLBptrStateFunc = STATE_init;
  Serial.println(F("STATE INIT"));
  FastLED.addLeds<WS2812B,PIN_LED_STRIP,GRB>(GLBledStripBufferMem, NUM_LEDS);



  Serial.println("Setup DHT22 ");
  if (GLBsensorDHT.setup(PIN_SENSOR_TEMP)!=0) {
      Serial.print("Failing dht22 setup on pin:");
      Serial.println(PIN_SENSOR_TEMP);
  }


  pinMode(PIN_LED_WHITE, OUTPUT);


  GLBsensorHC.setup();
  GLBsensorHC.trigger();  

  // TODO TEST
  mainTimers.setInterval(2000,GLBcallbackLogging);

  mainTimers.setInterval(7000,GLBcallbackLoggingTemp);

  mainTimers.setInterval(5000,GLBcallbackLoggingLedWhite);

  mainTimers.setInterval(10000,GLBcallbackLoggingLedStrip);

  mainTimers.setInterval(1000,GLBcallbackLoggingUltrasonic);



  tone(PIN_BUZZER, 500, 200);


}

//------------------------------------------------
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

//-------------------------------------------------
void processSerialInputString()
{
  strcpy(GLBauxString,GLBserialInputString);
  GLBserialInputString[0]=0;
  GLBserialInputStringReady = false;
  GLBledStrip.processCommands(GLBauxString);
}
//-------------------------------------------------
void STATE_init(void)
{
  Serial.println(F("DEBUG: inputBootString..."));
  strcpy_P(GLBauxString,(char*)inputBootString);
  GLBledStrip.processCommands(GLBauxString);

  GLBptrStateFunc=STATE_welcome;
  Serial.println(F("STATE INIT -> WELCOME"));
}
//-------------------------------------------------
void STATE_welcome(void)
{
  if (GLBledStrip.isIdle()) {
    GLBptrStateFunc=STATE_idle;
    Serial.println(F("STATE WELCOME -> IDLE"));
  }
}
//-------------------------------------------------
void STATE_idle(void)
{
  if (GLBserialInputStringReady){
    processSerialInputString();
    GLBptrStateFunc=STATE_LDcmd;
    Serial.println(F("STATE IDLE -> CMD"));
  }
}

//-------------------------------------------------
void STATE_LDcmd(void)
{
  if (GLBserialInputStringReady){
    processSerialInputString();
  }
  else if (GLBledStrip.isIdle()) {
    GLBptrStateFunc=STATE_idle;
    Serial.println(F("STATE LD CMD -> IDLE"));
  }
}



//-------------------------------------------------
//-------------------------------------------------
//-------------------------------------------------
void loop() { 
  


  //------------- INPUT REFRESHING ----------------
  GLBsensorDHT.refresh();
  int distance = GLBsensorHC.getLatestDistanceRead();
  if ((distance > 5) &&  (distance < 50)) {
    tone(PIN_BUZZER, 2000, 10);
  }
    
  //--------- TIME TO THINK MY FRIEND -------------
  // State machine as main controller execution
  GLBptrStateFunc();
  mainTimers.run();

  // ------------- OUTPUT REFRESHING ---------------
  GLBledStrip.refresh();
  FastLED.show();


}




