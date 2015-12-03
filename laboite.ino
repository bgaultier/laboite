/*

  laboite 3.9
 This Arduino firmware is part of laboite project https://laboite.cc/help
 It is a connected device displaying a lot of information (A LOT !) coming from an
 Internet server with a laboite web app deployed (e.g. https://laboite.cc/ ).
 
 Key Features:
 * Connects to laboite-webapp API to retrieve data
 * Parse the json received and display datas on the LED matrix
 * Indoor temperature (optionnal, uncomment SENSORS to enable it)
 * Automatic screen brightness adjusting  (optionnal, uncomment SENSORS to enable it)
 * Stop scrolling function (optionnal, uncomment SENSORS to enable it)
 
 Apps supported ( more info: https://laboite.cc/apps )
 * Time
 * Weather
 * Bus
 * Bikes
 * Energy
 * Waves
 * Messages
 * Coffees
 * Emails
 * RATP
 * Agenda
 * Parking
 * Metro
 
 Circuit:
 * Ethernet shield attached to pins 10, 11, 12, 13 (Arduino Yún support from v3.4)
 * Sure Electronics 3216 LED matrix attached to pins 4, 5, 6, 7
 * LDR, Thermistor and Button modules on A0, A1, A2 (optionnal, uncomment SENSORS to enable it)
 * Electric Imp Shield support
 
 created 15 Dec 2011
 by Baptiste Gaultier and Tanguy Ropitault
 modified 2 Dec 2015
 by Baptiste Gaultier
 
 This code is in the public domain.
 
 */

// uncomment if you want to enable debug
//#define DEBUG
// uncomment if you want to enable Ethernet
//#define ETHERNET
// uncomment if you want to enable dotmatrix
#define HT1632C
// uncomment if you want to enable classic sensors
//#define SENSORS
// uncomment if you want to enable the AVR Watchdog
#define WATCHDOG

#ifdef ETHERNET
#include <SPI.h>
#include <Ethernet.h>
#endif
#ifdef HT1632C
#include <ht1632c.h>
#endif
#ifdef WATCHDOG
#include <avr/wdt.h>
#endif


#ifndef ETHERNET
//This will be use to communicate with the Electric Imp
#include <SoftwareSerial.h>

SoftwareSerial impSerial(8, 9); // RX, TX
#endif

char serverName[] = "api.laboitestar.fr";    // your favorite API server running laboite-webapp https://github.com/bgaultier/laboite-webapp
char apikey[] = "bd2cecb5";              // your device API key

String currentLine = "";                 // string to hold the text from server

// Modular Apps code
// (uncomment only the apps you need, otherwise the sketch will be too big)
//#define BUS
//#define ENERGY
#define MESSAGES
//#define COFFEES
//#define AGENDA
#define PARKING
#define METRO
#define BUSSTOP
#define SLOTS


// Variables used to display infos
char hour[3];
char minutes[3];

// dotmatrix scrolling speed, higher=slower
byte speed = 50;

// laboite sleeping mode, true means screen off
boolean sleeping = false;

byte todayIcon;
char temperature[3];
byte color;

#ifdef SENSORS
char indoorTemperatureString[3];
byte indoorTemperature;
#endif

#ifdef BUS
char bus[3];
#endif


char bikes[3];

#ifdef SLOTS
char slots[3];
#endif

#ifdef ENERGY
byte energy[7];
#endif

#ifdef BUSSTOP
char route0[3];
char headsign0[32];
char departure0[3];
char route1[3];
char headsign1[32];
char departure1[3];
#endif

#ifdef EMAILS
char emails[3];
#endif

#ifdef MESSAGES
char message[140];
#endif

#ifdef COFFEES
char coffees[3];
#endif

#ifdef AGENDA
char eventStart[5];
char eventSummary[64];
#endif

#ifdef PARKING
boolean parkingOpen;
char parkingSpaces[4];
#endif

#ifdef METRO
char metroFailure[2];
#endif

// Parser variables
boolean readingTime = false;
boolean readingSpeed = false;
boolean readingSleepingMode = false;

boolean readingTodayIcon = false;
boolean readingTemperature = false;
boolean readingTomorrowIcon = false;
boolean readingLow = false;
boolean readingHigh = false;

#ifdef BUS
boolean readingBus = false;
#endif
boolean readingBikes = false;

#ifdef SLOTS
boolean readingSlots = false;
#endif

#ifdef ENERGY
boolean readingDay0 = false;
boolean readingDay1 = false;
boolean readingDay2 = false;
boolean readingDay3 = false;
boolean readingDay4 = false;
boolean readingDay5 = false;
boolean readingDay6 = false;
#endif

#ifdef PARKING
boolean readingParkingOpen = false;
boolean readingParkingSpaces = false;
#endif

#ifdef METRO
boolean readingMetroFailure = false;
#endif

#ifdef BUSSTOP
boolean readingRoute0 = false;
boolean readingRoute1 = false;
boolean readingHeadsign0 = false;
boolean readingHeadsign1 = false;
boolean readingDeparture0 = false;
boolean readingDeparture1 = false;
#endif

#ifdef MESSAGES
boolean readingMessage = false;
#endif

#ifdef COFFEES
boolean readingCoffees = false;
#endif

boolean readingEmails = false;

#ifdef AGENDA
boolean readingEventStart = false;
boolean readingEventSummary = false;
#endif

// Apps variables

boolean timeEnabled = false;
boolean busEnabled = false;
boolean bikesEnabled = false;
boolean emailsEnabled = false;
boolean weatherEnabled = false;
#ifdef ENERGY
boolean energyEnabled = false;
#endif
#ifdef MESSAGES
boolean messagesEnabled = false;
#endif
#ifdef COFFEES
boolean coffeesEnabled = false;
#endif
#ifdef AGENDA
boolean agendaEnabled = false;
#endif
#ifdef PARKING
boolean parkingEnabled = false;
#endif
#ifdef METRO
boolean metroEnabled = false;
#endif
#ifdef BUSSTOP
boolean busStopEnabled= false;
#endif

#ifdef SENSORS
// Sensors constants won't change. They're used here to 
// set pin numbers:
const byte ldrPin = A0;            // ldr used to adjust dotmatrix brightness
const byte thermistorPin = A1;     // thermistor used for indoor temperature
const byte buttonPin = A2;         // pushbutton used to start/stop scrolling

// Sensors variables
int brightnessValue = 0;              // value read from the LDR
int previousBrightnessValue = 512;    // previous value of brightness
#endif

#ifdef HT1632C
// initialize the dotmatrix with the numbers of the interface pins (data→7, wr→6, clk→4, cs→5)
ht1632c dotmatrix = ht1632c(&PORTD, 7, 6, 4, 5, GEOM_32x16, 2);
// uncomment if you are using an Arduino MEGA
//ht1632c dotmatrix = ht1632c(&PORTA, 0, 1, 3, 2, GEOM_32x16, 2);

// weather app sprites:
uint16_t sprites[5][9] =
{
  { 0x0100, 0x0100, 0x2008, 0x1390, 0x0440, 0x0820, 0x682c, 0x0820, 0x0440 },
  { 0x0000, 0x01c0, 0x0230, 0x1c08, 0x2208, 0x4004, 0x4004, 0x3ff8, 0x0000 },
  { 0x01c0, 0x0230, 0x1c08, 0x2208, 0x4004, 0x4004, 0x3ff8, 0x1500, 0x1500 },
  { 0x0000, 0x0000, 0x7ffe, 0x0000, 0x7ffe, 0x0000, 0x7ffe, 0x0000, 0x0000 },
  { 0x0540, 0x0380, 0x1110, 0x0920, 0x1ff0, 0x0920, 0x1110, 0x0380, 0x0540 }
};
#ifdef BUS
// bus app sprite:
uint16_t busSprite[9] = { 0x00fc, 0x0186, 0x01fe, 0x0102, 0x0102, 0x01fe, 0x017a, 0x01fe, 0x0084};
#endif
// bikes app sprite:
uint16_t bikeSprite[9] = { 0x020c, 0x0102, 0x008c, 0x00f8, 0x078e, 0x0ab9, 0x0bd5, 0x0891, 0x070e};
// slots app sprite:
uint16_t slotSprite[9] = { 0x0078, 0x00fc, 0x00cc, 0x00cc, 0x00fc, 0x0078, 0x0078, 0x0030, 0x0000};
// emails app sprite:
uint16_t emailSprite[6] = { 0x00fe, 0x0145, 0x0129, 0x0111, 0x0101, 0x00fe};
#ifdef COFFEES
// coffees app sprite
uint16_t coffeeSprite[8] = {0x4800, 0x2400, 0x4800, 0xff00, 0x8500, 0x8600, 0x8400, 0x7800};
#endif
#ifdef AGENDA
// agenda app sprite
uint16_t calendarSprite[8] = { 0b01111111, 0b01111111, 0b01000001, 0b01001001, 0b01001001, 0b01001001, 0b01000001, 0b01111111 };
#endif
#ifdef PARKING
// parking app sprite
uint16_t parkingSprite[8] =  { 0x01fc, 0x0106, 0x0132, 0x0132, 0x0106, 0x013c, 0x0120, 0x01e0 };
#endif
#ifdef METRO
// metro app sprite
uint16_t metroSprite[8] =  { 0x039c, 0x07fe, 0x0666, 0x0666, 0x0666, 0x0666, 0x0666, 0x0666 };
#endif

boolean scrolling = true;             // value modified when button is pressed

byte pwm = 15;                         // value output to the PWM (analog out)
#endif

void setup() {
  // reserve space:
  currentLine.reserve(256);
  
  // initialize serial:
  #ifdef DEBUG
  Serial.begin(9600);
  #endif
  
  // display a welcome message:
  #ifdef DEBUG
  Serial.println("laboite v3.9 starting...");
  #endif

  // attempt a DHCP connection:  
  #ifdef ETHERNET
  Ethernet.begin(mac);
  #endif
  
  #ifdef DEBUG
  #ifdef ETHERNET
  Serial.print("My address:");
  Serial.println(Ethernet.localIP());
  #endif
  #endif
  
  // enable the watchdog timer (8 seconds)
  #ifdef WATCHDOG
  wdt_enable(WDTO_8S);
  #endif
  
  #ifdef HT1632C
  // initialize dotmatrix:
  dotmatrix.clear();
  // dotmatrix brightness
  dotmatrix.pwm(pwm);
  // dotmatrix font
  dotmatrix.setfont(FONT_5x7);
  #endif
  
  #ifdef SENSORS
  pinMode(ldrPin, INPUT);
  pinMode(thermistorPin, INPUT);
  pinMode(buttonPin, INPUT);
  #endif
  
  #ifndef ETHERNET
  // set a slow data rate for the Imp
  impSerial.begin(1200);
  connectToServer();
  #endif
}

void loop()
{
  if (impSerial.available()) {
    parseJSON();
    
    #ifdef HT1632C
    // if !scrolling only time will be shown
    if(scrolling) {
      if(timeEnabled)
          printTime(0);
      // Reading the temperature in Celsius degrees and store in the indoorTemperature variable
      #ifdef TINKERKIT
      indoorTemperature = therm.readCelsius();
      itoa(indoorTemperature, indoorTemperatureString, 10);
      #endif
      #ifdef SENSORS
        itoa(getTemperature(), indoorTemperatureString, 10);
        #endif
        
        
        for (int x = 32; x > -162; x--) {
          adjustBrightness();
          
          // scroll through apps
          scrollFirstPanel(x);
          scrollSecondPanel(x);
          scrollThirdPanel(x);
          scrollFourthPanel(x);
          #ifdef AGENDA
          if(agendaEnabled)
            scrollFifthPanel(x);
          else {
          #endif
            if(x == -129) {
              dotmatrix.sendframe();
              break;
            }
          #ifdef AGENDA
          }
          #endif
          
          dotmatrix.sendframe();
          
          delay(speed);
          
        }
        
        #ifdef MESSAGES
        scrollSixthPanel();
        #endif
        
        #ifdef TINKERKIT
        if(button.read())
          scrolling = !scrolling;
        #endif
        #ifdef SENSORS
        if (digitalRead(buttonPin) == HIGH)
          scrolling = !scrolling;
        #endif
      }
  }
  else {
    if(sleeping)
      blinkPixel();
    if(scrolling) {
      // if you're not connected and you're scrolling
      // then attempt to connect again:
      connectToServer();
    }
    else {
      printTime(0);
      dotmatrix.sendframe();
      for (byte i = 0; i < 5; i++) {
        #ifdef WATCHDOG
        wdt_reset();
        #endif
        delay(4000);
      }
      #ifdef SENSORS
      if (digitalRead(buttonPin) == HIGH)
        scrolling = !scrolling;
      #endif
      connectToServer();
    }
    #endif
  }
}
