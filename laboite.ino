/*

  laboite v2.5
 
 Key Features:
 * Indoor temperature
 * 2 days weather forecasts
 * Automatic screen brightness adjusting
 * Clock synchronization (NTP)
 * Weather forecast icons (sunny, cloudy, rain, snow, fog)
 * Next bus arrival information from Keolis real-time API http://data.keolis-rennes.com/
 * Number of available bikes LE vélo STAR from Keolis real-time API
 * Number of unread mails
 * Energy history charts from past 7 days (emoncms)
 
 Circuit:
 * Ethernet shield attached to pins 10, 11, 12, 13
 * Sure Electronics 3216 LED matrix attached to pins 4, 5, 6, 7
 * TinkerKit LDR, Thermistor and Button modules on I0, I1, I2
 
 created 15 Dec 2011
 by Baptiste Gaultier and Tanguy Ropitault
 modified 22 Apr 2013
 by Baptiste Gaultier
 
 */
 
#include <SPI.h>
#include <Ethernet.h>
#include <ht1632c.h>
#include <TinkerKit.h>
#include <avr/wdt.h>

#define NODEBUG

// initialize the dotmatrix with the numbers of the interface pins (data→7, wr →6, clk→4, cs→5)
ht1632c dotmatrix = ht1632c(&PORTD, 7, 6, 4, 5, GEOM_32x16, 2);

TKLightSensor ldr(I0);             // ldr used to adjust dotmatrix brightness
TKThermistor therm(I1);            // thermistor used for indoor temperature
TKButton button(I2);               // button used to start/stop scrolling

boolean scrolling = true;          // value modified when button is pressed

int brightnessValue = 0;           // value read from the LDR
int previousBrightnessValue;       // previous value of brightness

byte pwm = 8;                      // value output to the PWM (analog out)

// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:
byte mac[] = { 0x90, 0xA2, 0xDA, 0x00, 0x65, 0xA4 };

// fill in an available IP address on your network here,
// for auto-configuration:
IPAddress ip(169, 254, 0, 64);
IPAddress subnet(255, 255, 0, 0);


// initialize the library instance:
EthernetClient client;

const int requestInterval = 16000;     // delay between requests

char serverName[] = "api.baptistegaultier.fr";  // Your favorite API server

// Variables used to parse the XML from emoncms

String currentLine = "";            // string to hold the text from server

// variables used to display infos
char hour[3];
char minutes[3];
byte todayIcon;
byte tomorrowIcon;
byte color;
char indoorTemperatureString[3];
byte indoorTemperature;
char temperature[3];
char low[3];
char high[3];
char nextBus[3];
byte kWhdHistory[7];

// weather forecast sprites:
uint16_t sprites[5][9] =
{
  { 0x0100, 0x0100, 0x2008, 0x1390, 0x0440, 0x0820, 0x682c, 0x0820, 0x0440 },
  { 0x0000, 0x01c0, 0x0230, 0x1c08, 0x2208, 0x4004, 0x4004, 0x3ff8, 0x0000 },
  { 0x01c0, 0x0230, 0x1c08, 0x2208, 0x4004, 0x4004, 0x3ff8, 0x1500, 0x1500 },
  { 0x0000, 0x0000, 0x7ffe, 0x0000, 0x7ffe, 0x0000, 0x7ffe, 0x0000, 0x0000 },
  { 0x0540, 0x0380, 0x1110, 0x0920, 0x1ff0, 0x0920, 0x1110, 0x0380, 0x0540 }
};

// bus sprite
uint16_t busSprite[9] = { 0x00fc, 0x0186, 0x01fe, 0x0102, 0x0102, 0x01fe, 0x017a, 0x01fe, 0x0084};

void setup() {
  // reserve space for the strings:
  currentLine.reserve(32);
  
  // Dotmatrix brightness and font
  dotmatrix.setfont(FONT_5x7);
  dotmatrix.pwm(pwm);
  
  // initialize serial:
  #ifdef DEBUG
  Serial.begin(9600);
  #endif
  // initialize dotmatrix:
  dotmatrix.clear();
  
  // display a welcome message:
  #ifdef DEBUG
  Serial.println("laboite v2.5 starting...");
  #endif
  
  Ethernet.begin(mac);
  /*
  // attempt a DHCP connection:
  if (!Ethernet.begin(mac)) {
    // if DHCP fails, start with a hard-coded address:
    Ethernet.begin(mac, ip, subnet);
  }
  */
  // print your local IP address:
  #ifdef DEBUG
  Serial.print("My address: ");
  Serial.println(Ethernet.localIP());
  #endif
  
  // Enable the watchdog timer (8 seconds)
  wdt_enable(WDTO_8S);
  
  // connect to API server:
  connectToServer();
}



void loop() {
  if (client.connected()) {
    if (client.available()) {
      parseXML();
      client.stop();
      
      if(scrolling) {
        printTime(0);
        // Reading the temperature in Celsius degrees and store in the indoorTemperature variable
        indoorTemperature = therm.getCelsius();
        itoa(indoorTemperature, indoorTemperatureString, 10);
        
        for (int x = 32; x > -96; x--) {
          adjustBrightness();
          
          scrollFirstPanel(x);
          scrollSecondPanel(x);
          scrollThirdPanel(x);
          scrollFourthPanel(x);
          
          dotmatrix.sendframe();
          
          if(x == -63) {
            waitAWhile();
            waitAWhile();
          }
          
          delay(50);
        }
        if(button.get())
          scrolling = !scrolling;
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
      
      if(button.get())
        scrolling = !scrolling;
      connectToServer();
    }
  }
}

