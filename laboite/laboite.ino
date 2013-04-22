/*

  laboite v2.4
 
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
  Serial.println("laboite v2.4 starting...");
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

void connectToServer() {
  // attempt to connect:
  #ifdef DEBUG
  Serial.print("Connecting to api.baptistegaultier.fr...");
  #endif
  if (client.connect(serverName, 80))
  {
    #ifdef DEBUG
    Serial.println("Making HTTP request...");
    #endif
    
    // make HTTP GET request to API server:
    client.println("GET /laboite.xml HTTP/1.1");
    client.println("Host: api.baptistegaultier.fr");
    client.println("User-Agent: Arduino/1.0");
    client.println();
  }
}

void printTemperature(int x, char firstDigit, char secondDigit, byte color)
{
  if(secondDigit == '\0') {
    secondDigit = firstDigit;
    firstDigit = ' ';
  }
  dotmatrix.putchar(x, 9, firstDigit, color);
  dotmatrix.putchar(x+5, 9, secondDigit, color);
  dotmatrix.putchar(x+10, 9, '*', color);
  
  // print the results to the serial monitor:
  #ifdef DEBUG
  //Serial.print("brightness : " );                      
  //Serial.println(brightnessValue);
  #endif
}

boolean parseXML() {
  String content = "";
  
  // fetch time
  content = getXMLElementContent("<time>");
  hour[0] = content.charAt(0);
  hour[1] = content.charAt(1);
  hour[3] = '\0';
  
  minutes[0] = content.charAt(3);
  minutes[1] = content.charAt(4);
  minutes[3] = '\0';
  
  // fetch next bus arrival
  content = getXMLElementContent("<nextbus>");
  nextBus[0] = content.charAt(0);
  nextBus[1] = content.charAt(1);
  nextBus[2] = '\0';
  
  // fetch energy consumption history data
  for(int i = 0; i < 7; i++) {
    content = getXMLElementContent(String("<day") + i + String(">"));
    kWhdHistory[i] = stringToInt(content);
  }
  
  // fetch weather icon (today)
  content = getXMLElementContent("<today>");
  todayIcon = stringToInt(content);
  
  // fetch current outdoor temperature
  content = getXMLElementContent("<temperature>");
  temperature[0] = content.charAt(0);
  temperature[1] = content.charAt(1);
  temperature[2] = '\0';
  
  // fetch weather icon (tomorrow)
  content = getXMLElementContent("<tomorrow>");
  tomorrowIcon = stringToInt(content);
  
  // fetch weather icon (tomorrow)
  content = getXMLElementContent("<low>");
  low[0] = content.charAt(0);
  low[1] = content.charAt(1);
  low[2] = '\0';
  
  content = getXMLElementContent("<high>");
  high[0] = content.charAt(0);
  high[1] = content.charAt(1);
  high[2] = '\0';
}

String getXMLElementContent(String tag) {
  String content = "";               // string to hold XML element's content
  boolean readingContent = false;    // if you're currently reading the content
  boolean tagFound = false;          // if you have found the tag
  
  while(client.available() && !tagFound) {
    // read incoming bytes:
    char inChar = client.read();

    // add incoming byte to end of line:
    currentLine += inChar;

    // if you get a newline, clear the line:
    if (inChar == '\n') {
      currentLine = "";
    }
    // if the current line ends with <tag>, it will
    // be followed by the XML element's content:
    if ( currentLine.endsWith(tag)) {
      // element is beginning. Clear the content string:
      readingContent = true;
      content = "";
    }
    // if you're currently reading the bytes of a tweet,
    // add them to the content String:
    if (readingContent) {
      if (inChar != '<') {
        if (inChar != '>')
          content += inChar;
      }
      else {
        // if you got a "<" character,
        // you've reached the end of the content:
        readingContent = false;
        Serial.print("Tag ");
        Serial.print(tag);
        Serial.println(" found !");
        tagFound = true;
      }
    }
  }
  return content;
}

void printTime(int x) {
  dotmatrix.putchar(x+5, 0, hour[0], GREEN);
  dotmatrix.putchar(x+10, 0, hour[1], GREEN);
  dotmatrix.putchar(x+14, 0, ':', GREEN);
  dotmatrix.putchar(x+18, 0, minutes[0], GREEN);
  dotmatrix.putchar(x+23, 0, minutes[1], GREEN);
}

void adjustBrightness() {
  // reset the watchdog timer
  wdt_reset();
  // read the analog in value:
  brightnessValue = (ldr.get() + previousBrightnessValue) / 2;
  pwm = map(brightnessValue, 0, 1023, 0, 15);
  dotmatrix.pwm(pwm);
  previousBrightnessValue = brightnessValue;
  
  // check if scrolling button has been pressed
  if(button.get())
    scrolling = !scrolling;
}

void waitAWhile() {
  for (int i = 0; i < 16; i++) {
    adjustBrightness();
    delay(50);
  }
}

int stringToInt(String string) {
  char buffer[8];
  string.toCharArray(buffer, string.length()+1);
  return atoi(buffer);
}

void scrollFirstPanel(int x) {
  // first panel : current weather condition 32→0
  if(x > -16) {
    dotmatrix.putchar(x+12, 9, ' ', RED);
    color = todayIcon == 0 ? color = ORANGE : color = RED;
    dotmatrix.putbitmap(x, 7, sprites[todayIcon],16,9, color);
  }
  
  if(x >= 0) {
    printTemperature(x+17, temperature[0], temperature[1], RED);
    dotmatrix.sendframe();
  }
  
  if(x == 0) {
    waitAWhile();
    printTemperature(x+17, indoorTemperatureString[0], indoorTemperatureString[1], ORANGE);
    dotmatrix.sendframe();
    waitAWhile();
  }
}

void scrollSecondPanel(int x) {
  // second panel : tomorrow weather condition 0→-32
  if(x <= 1 && x >= -48) {
    dotmatrix.putchar(x+44, 9, ' ', RED);
    color = tomorrowIcon == 0 ? color = ORANGE : color = RED;
    dotmatrix.putbitmap(x+32, 7, sprites[tomorrowIcon],16,9, color);
  }
  
  if(x >= -32 && x < 0) {
    printTemperature(x+17, indoorTemperatureString[0], indoorTemperatureString[1], ORANGE);
    printTemperature(x+49, low[0], low[1], RED);
    dotmatrix.sendframe();
  }
  
  if(x == -32) {
    waitAWhile();
    printTemperature(x+49, high[0], high[1], GREEN);
    dotmatrix.sendframe();
    waitAWhile();
  }
}

void scrollThirdPanel(int x) {
  //third panel : email and next bus arrival
  if(x >= -63 && x < -32) {
    printTemperature(x+49, high[0], high[1], GREEN);
    printTime(x+32);
  }
}

void scrollFourthPanel(int x) {
  //fourth panel : bikes available and energy chart
  if(x <= -33) {
    // bus
    if(nextBus[0] == '-')
      dotmatrix.putchar(x+80, 2, '<', GREEN);
    else
      dotmatrix.putchar(x+80, 2, nextBus[0], GREEN);
      
    if(nextBus[1] == '\0')
      dotmatrix.putchar(x+85, 2, '\'', GREEN);
    else {
      dotmatrix.putchar(x+85, 2, nextBus[1], GREEN);
      dotmatrix.putchar(x+90, 2, '\'', GREEN);
    }
    
    dotmatrix.putbitmap(x+67, 0, busSprite, 9, 9, ORANGE);
    
    printTime(x+95);
    
    for(int i = 0; i < 7; i++) {
      drawChart(x + 65 + (i*4), kWhdHistory[i]);
    }
  }
}
            
            
void drawChart(byte x, byte height) {
  dotmatrix.rect(x, 16-height, x+2, 15, GREEN);
  if(height > 2)
    dotmatrix.line(x+1, 17-height, x+1, 14, BLACK);
  dotmatrix.line(x+3, 16-height, x+3, 15, BLACK);
}
