/*

  laboite v3.1
 This Arduino firmware is part of laboite project http://laboite.cc/help
 It is a connected device displaying a lot of information (A LOT !) coming from an
 Internet server with a laboite web app deployed (e.g. http://laboite.cc/ ).
 
 Key Features:
 * Connects to laboite-webapp to retrive apps data
 * Indoor temperature
 * Automatic screen brightness adjusting
 * Stop scrolling function
 
 Apps supported ( more info: http://laboite.cc/apps )
 * Time
 * Weather
 * Bus
 * Bikes
 * Energy
 * Waves
 * Messages
 * Coffees
 * Emails
 
 Circuit:
 * Ethernet shield attached to pins 10, 11, 12, 13
 * Sure Electronics 3216 LED matrix attached to pins 4, 5, 6, 7
 * TinkerKit LDR, Thermistor and Button modules on I0, I1, I2
 
 created 15 Dec 2011
 by Baptiste Gaultier and Tanguy Ropitault
 modified 24 Jul 2013
 by Baptiste Gaultier
 
 This code is in the public domain.
 
 */
// uncomment if you want to enable debug
//#define DEBUG
// uncomment if you want to enable dotmatrix
#define HT1632C
// uncomment if you want to enable TinkerKit! sensors
//#define SENSORS

#include <SPI.h>
#include <Ethernet.h>
#ifdef SENSORS
#include <TinkerKit.h>
#endif
#ifdef HT1632C
#include <ht1632c.h>
#endif
#include <avr/wdt.h>

// enter a MAC address and IP address for your controller below.
byte mac[] = { 0x90, 0xA2, 0xDA, 0x00, 0xE5, 0x91 };

// fill in an available IP address on your network here,
// for auto-configuration:
IPAddress ip(169, 254, 0, 64);
IPAddress subnet(255, 255, 0, 0);

// initialize the library instance:
EthernetClient client;

const int requestInterval = 16000;       // delay between requests

char serverName[] = "api.laboite.cc";    // your favorite API server running laboite-webapp https://github.com/bgaultier/laboite-webapp
char apikey[] = "61c119ce";              // your device API key

String currentLine = "";                 // string to hold the text from server

// variables used to display infos
char hour[3];
char minutes[3];
char bus[3];
char bikes[3];
char emails[3];
char coffees[3];
byte todayIcon;
byte tomorrowIcon;
byte color;
char indoorTemperatureString[3];
byte indoorTemperature;
char temperature[3];
char low[3];
char high[3];
byte energy[7];
char message[140];

// parser variables
boolean readingTime = false;
boolean readingBus = false;
boolean readingBikes = false;
boolean readingEmails = false;
boolean readingCoffees = false;
boolean readingTodayIcon = false;
boolean todayIconRead = false;
boolean readingTemperature = false;
boolean readingTomorrowIcon = false;
boolean readingLow = false;
boolean readingHigh = false;
boolean readingDay0 = false;
boolean readingDay1 = false;
boolean readingDay2 = false;
boolean readingDay3 = false;
boolean readingDay4 = false;
boolean readingDay5 = false;
boolean readingDay6 = false;
boolean readingMessage = false;

// apps variables
boolean timeEnabled = false;
boolean busEnabled = false;
boolean bikesEnabled = false;
boolean emailsEnabled = false;
boolean weatherEnabled = false;
boolean coffeesEnabled = false;
boolean energyEnabled = false;
boolean messagesEnabled = false;

#ifdef SENSORS
TKLightSensor ldr(I0);             // ldr used to adjust dotmatrix brightness
TKThermistor therm(I1);            // thermistor used for indoor temperature
TKButton button(I2);               // button used to start/stop scrolling
#endif


#ifdef HT1632C
// initialize the dotmatrix with the numbers of the interface pins (data→7, wr→6, clk→4, cs→5)
ht1632c dotmatrix = ht1632c(&PORTD, 7, 6, 4, 5, GEOM_32x16, 2);

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
// coffees app sprite
uint16_t coffeeSprite[8] = {0x4800, 0x2400, 0x4800, 0xff00, 0x8500, 0x8600, 0x8400, 0x7800};

int brightnessValue = 0;              // value read from the LDR
int previousBrightnessValue = 512;    // previous value of brightness

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
  Serial.println("laboite v3.1 starting...");
  #endif

  // attempt a DHCP connection:
  #ifdef DEBUG
  Serial.println("Attempting to get an IP address using DHCP:");
  #endif
  
  if (!Ethernet.begin(mac)) {
    // if DHCP fails, start with a hard-coded address:
    #ifdef DEBUG
    Serial.println("failed to get an IP address using DHCP, trying manually");
    #endif
    Ethernet.begin(mac, ip, subnet);
  }
  #ifdef DEBUG
  Serial.print("My address:");
  Serial.println(Ethernet.localIP());
  #endif
  
  // enable the watchdog timer (8 seconds)
  wdt_enable(WDTO_8S);
  
  #ifdef HT1632C
  // initialize dotmatrix:
  dotmatrix.clear();
  // dotmatrix brightness
  dotmatrix.pwm(pwm);
  // dotmatrix font
  dotmatrix.setfont(FONT_5x7);
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
        #ifdef SENSORS
        indoorTemperature = therm.readCelsius();
        itoa(indoorTemperature, indoorTemperatureString, 10);
        #endif
        
        // compute the max number of pixels we have to scroll
        int maxScroll = -130;
        if(!weatherEnabled)
          maxScroll+=32;
        if(!busEnabled && !bikesEnabled)
          maxScroll+=32;
        if(!coffeesEnabled && !energyEnabled)
          maxScroll+=32;
        
        
        for (int x = 32; x > maxScroll; x--) {
          adjustBrightness();
          
          // scroll through apps
          scrollFirstPanel(x);
          scrollSecondPanel(x);
          scrollThirdPanel(x);
          scrollFourthPanel(x);
          
          dotmatrix.sendframe();
          
          if(x == -63 || x == -95) {
            waitAWhile();
            waitAWhile();
          }         
          
          delay(50);
        }
        scrollFifthPanel();
        
        #ifdef SENSORS
        if(button.read())
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
        wdt_reset();
        delay(requestInterval/4);
      }
      #ifdef SENSORS
      if(button.read())
        scrolling = !scrolling;
      #endif
      connectToServer();
      #endif
    }
  }
}
