/*

  laboite v3.0
 This Arduino firmware is part of laboite project http://laboite.cc/help
 It is a connected device displaying a lot of information (A LOT !) coming from an
 Internet server with a laboite web app deployed (e.g. http://laboite.cc/ ).
 
 Key Features:
 * Connects to laboite-webapp to retrive apps data
 * Indoor temperature
 * Automatic screen brightness adjusting
 * Stop scrolling function
 
 Apps supported (more infos here http://laboite.cc/apps )
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
 
#include <SPI.h>
#include <Ethernet.h>
#include <avr/wdt.h>


// Enter a MAC address and IP address for your controller below.
byte mac[] = { 0x90, 0xA2, 0xDA, 0x00, 0x65, 0xA4 };

// fill in an available IP address on your network here,
// for auto-configuration:
IPAddress ip(169, 254, 0, 64);
IPAddress subnet(255, 255, 0, 0);

// initialize the library instance:
EthernetClient client;

const int requestInterval = 16000;       // delay between requests

char serverName[] = "api.laboite.cc";    // your favorite API server running laboite-webapp https://github.com/bgaultier/laboite-webapp
char apikey[] = "964de680";              // your device API key

// uncomment if you want to enable debug
#define DEBUG

String currentLine = "";                 // string to hold the text from server

// variables used to display infos
char hour[3];
char minutes[3];
char bus[3];
char bikes[3];
char emails[3];
byte todayIcon;
byte tomorrowIcon;
byte color;
char indoorTemperatureString[3];
byte indoorTemperature;
char temperature[3];
char low[3];
char high[3];

// parser variables
boolean readingTime = false;
boolean readingBus = false;
boolean readingBikes = false;
boolean readingEmails = false;
boolean readingTodayIcon = false;
boolean todayIconRead = false;
boolean readingTemperature = false;
boolean readingTomorrowIcon = false;
boolean readingLow = false;
boolean readingHigh = false;

// apps variables
boolean timeEnabled = false;
boolean busEnabled = false;
boolean bikesEnabled = false;
boolean emailsEnabled = false;
boolean weatherEnabled = false;


void setup() {
  // reserve space:
  currentLine.reserve(128);
  
  // initialize serial:
  
  #ifdef DEBUG
  Serial.begin(9600);
  #endif
  
  // display a welcome message:
  #ifdef DEBUG
  Serial.println("laboite v3.0 starting...");
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
    }
  }
}
