/*

  laboite v3.4
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
 
 Circuit:
 * Ethernet shield attached to pins 10, 11, 12, 13 (Arduino Yún support from v3.4)
 * Sure Electronics 3216 LED matrix attached to pins 4, 5, 6, 7
 * LDR, Thermistor and Button modules on A0, A1, A2 (optionnal, uncomment SENSORS to enable it)
 
 created 15 Dec 2011
 by Baptiste Gaultier and Tanguy Ropitault
 modified 31 Mar 2015
 by Baptiste Gaultier
 
 This code is in the public domain.
 
 */

// uncomment if you want to enable debug
//#define DEBUG
// uncomment if you want to enable Ethernet
#define ETHERNET
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

#ifdef ETHERNET
// enter a MAC address and IP address for your controller below.
byte mac[] = { 0x90, 0xA2, 0xDA, 0x00, 0xE5, 0x91 };

// initialize the library instance:
EthernetClient client;
#endif

const int requestInterval = 16000;       // delay between requests

char serverName[] = "api.laboite.cc";    // your favorite API server running laboite-webapp https://github.com/bgaultier/laboite-webapp
char apikey[] = "964de680";              // your device API key

String currentLine = "";                 // string to hold the text from server

// Modular Apps code
// (uncomment only the apps you need, otherwise the sketch will be too big)
#define ENERGY
#define MESSAGES
#define COFFEES
#define AGENDA


// Variables used to display infos

char hour[3];
char minutes[3];

byte todayIcon;
char temperature[3];
byte tomorrowIcon;
char low[3];
char high[3];
byte color;

#ifdef SENSORS
char indoorTemperatureString[3];
byte indoorTemperature;
#endif

char bus[3];

char bikes[3];

#ifdef ENERGY
byte energy[7];
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

char eventStart[5];
char eventSummary[64];

// Parser variables
boolean readingTime = false;

boolean readingTodayIcon = false;
boolean readingTemperature = false;
boolean readingTomorrowIcon = false;
boolean readingLow = false;
boolean readingHigh = false;

boolean readingBus = false;
boolean readingBikes = false;

#ifdef ENERGY
boolean readingDay0 = false;
boolean readingDay1 = false;
boolean readingDay2 = false;
boolean readingDay3 = false;
boolean readingDay4 = false;
boolean readingDay5 = false;
boolean readingDay6 = false;
#endif

boolean readingMessage = false;

#ifdef COFFEES
boolean readingCoffees = false;
#endif

boolean readingEmails = false;

boolean readingEventStart = false;
boolean readingEventSummary = false;


// Apps variables

boolean timeEnabled = false;
boolean busEnabled = false;
boolean bikesEnabled = false;
boolean emailsEnabled = false;
boolean weatherEnabled = false;
#ifdef ENERGY
boolean energyEnabled = false;
#endif
boolean messagesEnabled = false;
#ifdef COFFEES
boolean coffeesEnabled = false;
#endif
#ifdef AGENDA
boolean agendaEnabled = false;
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
// bus app sprite:
uint16_t busSprite[9] = { 0x00fc, 0x0186, 0x01fe, 0x0102, 0x0102, 0x01fe, 0x017a, 0x01fe, 0x0084};
// bikes app sprite:
uint16_t bikeSprite[9] = { 0x020c, 0x0102, 0x008c, 0x00f8, 0x078e, 0x0ab9, 0x0bd5, 0x0891, 0x070e};
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
  Serial.println("laboite v3.4 starting...");
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
  
  // connect to API server:
  connectToServer();
}


void loop()
{
  if (client.connected()) {
    if (client.available()) {
      // parse json file coming from API server
      parseJSON();
      client.stop();
      
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
            if(x == -129) {
              dotmatrix.sendframe();
              break;
            }
          }
          
          dotmatrix.sendframe();
          
          delay(30);
          
        }
        #endif
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
  }
  else {
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
        delay(requestInterval/4);
      }
      #ifdef SENSORS
      if (digitalRead(buttonPin) == HIGH)
        scrolling = !scrolling;
      #endif
      connectToServer();
      #endif
    }
  }
}
