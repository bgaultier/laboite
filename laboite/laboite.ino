/*

 laboite v2.0
 
 Key Features:
 * Indoor Temperature
 * 2 days forecasts
 * Automatic screen brightness adjusting
 * Automatic time (NTP)
 * Weather forecast icons (sunny, cloudy, rain, snow, fog)
 * Next bus arrival information from Keolis gtfs
 
 Circuit:
 * Ethernet shield attached to pins 10, 11, 12, 13
 * Sure Electronics 3216 LED matrix attached to pins 4, 5, 6, 7
 * TinkerKit LDR, Thermistor and Touch modules on I0, I1, I2
 
 created 15 Dec 2011
 by Baptiste Gaultier and Tanguy Ropitault
 modified 9 Apr 2012
 by Baptiste Gaultier
 
 */
 
 
#include <SPI.h>
#include <Ethernet.h>
#include <ht1632c.h>
#include <TinkerKit.h>

#define MAX_STOPS 8

int timetable[24][MAX_STOPS] =
{
  {},
  {},
  {},
  {},
  {},
  {31},
  {11, 29, 43},
  {3, 16, 23, 30, 37, 44, 52, 58},
  {06, 13, 21, 29, 37, 46, 55},
  {4, 12, 21, 30, 39, 48, 56},
  {5, 14, 22, 30, 38, 47, 56},
  {5, 14, 23, 32, 40, 48, 56},
  {4, 13, 22, 31, 41, 50, 59},
  {9, 18, 27, 36, 45, 54},
  {3, 12, 21, 28, 37, 46, 54},
  {3, 12, 21, 30, 39, 48, 56},
  {4, 9, 16, 24, 32, 40, 48, 56},
  {4, 13, 22, 31, 37, 43, 51},
  {0, 8, 16, 24, 32, 40, 48, 56},
  {4, 13, 23, 33, 43, 53},
  {4, 15, 30, 45},
  {0, 14, 29, 44, 59},
  {19, 34, 49},
  {4, 19, 34, 54}
};

// initialize the dotmatrix with the numbers of the interface pins (data→7, wr →6, clk→4, cs→5)
ht1632c dotmatrix = ht1632c(&PORTD, 7, 6, 4, 5, GEOM_32x16, 2);

TKLightSensor ldr(I0);    // ldr used to adjust dotmatrix brightness
//TKThermistor therm(I1);   // thermistor used for indoor temperature
TKTouchSensor touch(I2);  // button used to start/stop scrolling

boolean scrolling = true; // value modified when touch sensor pressed

int brightnessValue = 0; // value read from the LDR
byte pwm = 6;            // value output to the PWM (analog out)

char hour[3];
char minutes[3];
char nextBusString[3];
byte todayIcon;
byte tomorrowIcon;
byte color;
char indoorTemperatureString[3];
byte indoorTemperature;

// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:
byte mac[] = { 0x90, 0xA2, 0xDA, 0x00, 0x65, 0xA4 };

// fill in an available IP address on your network here,
// for auto-configuration:
//IPAddress ip(169, 254, 0, 64);
IPAddress ip(10, 35, 128, 111);
IPAddress subnet(255, 255, 255, 0);

// initialize the library instance:
EthernetClient client;

const int requestInterval = 10000;  // delay between requests

IPAddress server(192, 108, 119 ,4);              // Your favorite api server IP address

boolean requested;                   // whether you've made a request since connecting
long lastAttemptTime = 0;            // last time you connected to the server, in milliseconds

// Variables used to parse the XML from emoncms

String currentLine = "";             // string to hold the text from server
String content = "";
char temperature[3];
char low[3];
char high[3];

boolean readingTime = false;
boolean readingTodayIcon = false;
boolean readingTemperature = false;
boolean readingTomorrowIcon = false;
boolean readingLow = false;
boolean readingHigh = false;
boolean readingInstantaneous = false;
boolean readingDay0 = false;
boolean readingDay1 = false;
boolean readingDay2 = false;
boolean readingDay3 = false;
boolean readingDay4 = false;
boolean readingDay5 = false;
boolean readingDay6 = false;

int day0;
int day1;
int day2;
int day3;
int day4;
int day5;
int day6;


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
  currentLine.reserve(128);
  content.reserve(8);
  
  // Dotmatrix brightness
  dotmatrix.pwm(8);
  
  // initialize serial:
  //Serial.begin(9600);
  // initialize dotmatrix:
  dotmatrix.clear();
  
  // display a welcome message:
  //Serial.println("laboite v2.0 starting...");
  
  // attempt a DHCP connection:
  if (!Ethernet.begin(mac)) {
    // if DHCP fails, start with a hard-coded address:
    Ethernet.begin(mac, ip, subnet);
  }
  
  // print your local IP address:
  /*Serial.print("My address:");
  Serial.println(Ethernet.localIP());*/
  // connect to API server:
  connectToServer();
}



void loop()
{
  if (client.connected()) {
    if (client.available()) {
      // read incoming bytes:
      char inChar = client.read();

      // add incoming byte to end of line:
      currentLine += inChar; 

      // if you get a newline, clear the line:
      if (inChar == '\n') {
        currentLine = "";
      }
      
      if ( currentLine.endsWith("<time>")) {
        readingTime = true; 
        content = "";
      }

      if (readingTime) {
        if (inChar != '<') {
          if (inChar != '>')
            content += inChar;
        } 
        else {
          readingTime = false;
          hour[0] = content.charAt(0);
          hour[1] = content.charAt(1);
          hour[3] = '\0';
          
          minutes[0] = content.charAt(3);
          minutes[1] = content.charAt(4);
          minutes[3] = '\0';
        }
      }
      
      if ( currentLine.endsWith("<today>")) {
        readingTodayIcon = true; 
        content = "";
      }

      if (readingTodayIcon) {
        if (inChar != '<') {
          if (inChar != '>')
            content += inChar;
        }
        else {
          readingTodayIcon = false;
          todayIcon = stringToInt(content);
        }
      }
      
      if ( currentLine.endsWith("<temperature>")) {
        readingTemperature = true; 
        content = "";
      }

      if (readingTemperature) {
        if (inChar != '<') {
          if (inChar != '>')
            content += inChar;
        } 
        else {
          readingTemperature = false;
          temperature[0] = content.charAt(0);
          temperature[1] = content.charAt(1);
          temperature[2] = '\0';
        }
      }
      
      if (currentLine.endsWith("<tomorrow>")) {
        readingTomorrowIcon = true; 
        content = "";
      }

      if (readingTomorrowIcon) {
        if (inChar != '<') {
          if (inChar != '>')
            content += inChar;
        }
        else {
          readingTomorrowIcon = false;
          tomorrowIcon = stringToInt(content);
        }
      }
      
      if (currentLine.endsWith("<low>")) {
        readingLow = true; 
        content = "";
      }

      if (readingLow) {
        if (inChar != '<') {
          if (inChar != '>')
            content += inChar;
        }
        else {
          readingLow = false;
          low[0] = content.charAt(0);
          low[1] = content.charAt(1);
          low[2] = '\0';
        }
      }
      
      if (currentLine.endsWith("<high>")) {
        readingHigh = true; 
        content = "";
      }

      if (readingHigh) {
        if (inChar != '<') {
          if (inChar != '>')
            content += inChar;
        } 
        else {
          readingHigh = false;
          high[0] = content.charAt(0);
          high[1] = content.charAt(1);
          high[2] = '\0';
        }
      }

      if ( currentLine.endsWith("<day0>")) {
        readingDay0 = true; 
        content = "";
      }

      if (readingDay0) {
        if (inChar != '<') {
          if (inChar != '>')
            content += inChar;
        } 
        else {
          readingDay0 = false;
          day0 = stringToInt(content);
        }
      }

      if ( currentLine.endsWith("<day1>")) {
        readingDay1 = true; 
        content = "";
      }

      if (readingDay1) {
        if (inChar != '<') {
          if (inChar != '>')
            content += inChar;
        } 
        else {
          readingDay1 = false;
          day1 = stringToInt(content);
        }
      }

      if ( currentLine.endsWith("<day2>")) {
        readingDay2 = true; 
        content = "";
      }

      if (readingDay2) {
        if (inChar != '<') {
          if (inChar != '>')
            content += inChar;
        } 
        else {
          readingDay2 = false;
          day2 = stringToInt(content);
        }
      }

      if ( currentLine.endsWith("<day3>")) {
        readingDay3 = true; 
        content = "";
      }

      if (readingDay3) {
        if (inChar != '<') {
          if (inChar != '>')
            content += inChar;
        } 
        else {
          readingDay3 = false;
          day3 = stringToInt(content);
        }
      }

      if ( currentLine.endsWith("<day4>")) {
        readingDay4 = true; 
        content = "";
      }

      if (readingDay4) {
        if (inChar != '<') {
          if (inChar != '>')
            content += inChar;
        } 
        else {
          readingDay4 = false;
          day4 = stringToInt(content);
        }
      }

      if ( currentLine.endsWith("<day5>")) {
        readingDay5 = true; 
        content = "";
      }

      if (readingDay5) {
        if (inChar != '<') {
          if (inChar != '>')
            content += inChar;
        } 
        else {
          readingDay5 = false;
          day5 = stringToInt(content);
        }
      }
      if ( currentLine.endsWith("<day6>")) {
        readingDay6 = true; 
        content = "";
      }

      if (readingDay6) {
        if (inChar != '<') {
          if (inChar != '>')
            content += inChar;
        } 
        else {
          // if you got a ">" character, you've
          // reached the end of the XML
          readingDay6 = false;
          day6 = stringToInt(content);
          
          dotmatrix.setfont(FONT_5x7);
          
          dotmatrix.putchar(5, 0, hour[0], GREEN);
          dotmatrix.putchar(10, 0, hour[1], GREEN);
          dotmatrix.putchar(14, 0, ':', GREEN);
          dotmatrix.putchar(18, 0, minutes[0], GREEN);
          dotmatrix.putchar(23, 0, minutes[1], GREEN);
          
          dotmatrix.sendframe();
          
          if(scrolling)
          {
            // Reading the temperature in Celsius degrees and store in the indoorTemperature variable
            //indoorTemperature = therm.getCelsius();
            //itoa (indoorTemperature, indoorTemperatureString, 10);
            
            for (int x = 32; x > -64; x--)
            {
              dotmatrix.putchar(x+12, 9, ' ', RED);
              
              if(todayIcon == 0)
                color = ORANGE;
              else
                color = RED;
              dotmatrix.putbitmap(x, 7, sprites[todayIcon],16,9, color);
              
              dotmatrix.putchar(x+12+32, 9, ' ', RED);
              
              if(tomorrowIcon == 0)
                color = ORANGE;
              else
                color = RED;
              dotmatrix.putbitmap(x+32, 7, sprites[tomorrowIcon],16,9, color);
              
              dotmatrix.sendframe();
              
              if(x >= 0)
              {
                printTemperature(x+17, temperature[0], temperature[1], RED);
                dotmatrix.sendframe();
              }
              
              if(x >= -32 && x < 0)
              {
                //printTemperature(x+17, indoorTemperatureString[0], indoorTemperatureString[1], ORANGE);
                printTemperature(x+17, temperature[0], temperature[1], RED);
                printTemperature(x+49, low[0], low[1], RED);
                dotmatrix.sendframe();
              }
              
              if(x >= -63 && x < -32) {
                printTemperature(x+49, high[0], high[1], GREEN);
                dotmatrix.sendframe();
              }
              
              
              delay(50);
              
              if(x == 0)
              {
                delay(800);
                printTemperature(x+17, temperature[0], temperature[1], RED);
                dotmatrix.sendframe();
                delay(800);
              }
              
              if(x == -32)
              {
                delay(800);
                printTemperature(x+49, high[0], high[1], GREEN);
                dotmatrix.sendframe();
                delay(800);
              }
            }
            
            itoa(nextBus(atoi(hour), atoi(minutes)), nextBusString, 10);
            
            // next bus
            for (int x = 32; x > -56; x--)
            {
              // read the analog in value:
              brightnessValue = ldr.get();
              pwm = map(brightnessValue, 0, 1023, 0, 15);
              dotmatrix.pwm(pwm);
              
              dotmatrix.putchar(x+11, 10, ' ', GREEN);
              
              dotmatrix.putbitmap(x+1, 7, busSprite, 9, 9, ORANGE);          
              dotmatrix.putchar(x+11, 9, nextBusString[0], GREEN);
              
              if(nextBusString[1] == '\0')
              {
                dotmatrix.putchar(x+5+11, 9, '\'', GREEN);
              }
              else
              {
                dotmatrix.putchar(x+5+11, 9, nextBusString[1], GREEN);
                dotmatrix.putchar(x+10+11, 9, '\'', GREEN);
              }
              
              drawChart(x+2+24, day6);
              drawChart(x+6+24, day5);
              drawChart(x+10+24, day4);
              drawChart(x+14+24, day3);
              drawChart(x+18+24, day2);
              drawChart(x+22+24, day1);
              drawChart(x+26+24, day0);
              
              dotmatrix.sendframe();
              
              delay(50);
              
              if(x == 6)
                delay(800);
              if(x == -23)
                delay(2400);
            }
          }
          
          // close the connection to the server:
          client.stop(); 
        }
      }
    }   
  }
  else if (millis() - lastAttemptTime > requestInterval) {
    // if you're not connected, and ten seconds have passed since
    // your last connection, then attempt to connect again:
    if(touch.get())
      scrolling = !scrolling;
    connectToServer();
  }
}

void connectToServer()
{
  // attempt to connect:
  /*Serial.print("Connecting to ");
  Serial.print(serverName);
  Serial.println("...");*/
  if (client.connect(server, 80))
  {
    //Serial.println("Making HTTP request...");
    // make HTTP GET request to API server:
    client.println("GET /feed/arduino.xml?&apikey=8f82c752a93a8656d8e16858e8596c5b&id=2 HTTP/1.1");
    client.println("HOST: smartb.labo4g.enstb.fr");
    client.println("User-Agent: Arduino/1.0");
    client.println();
  }
  // note the time of this connect attempt:
  lastAttemptTime = millis();
}

void printTemperature(int x, char firstDigit, char secondDigit, byte color)
{
  if(firstDigit == '0')
    firstDigit = ' ';
  dotmatrix.putchar(x, 9, firstDigit, color);
  dotmatrix.putchar(x+5, 9, secondDigit, color);
  dotmatrix.putchar(x+10, 9, '*', color);
  
  // read the analog in value:
  brightnessValue = ldr.get();
  pwm = map(brightnessValue, 0, 1023, 0, 15);
  dotmatrix.pwm(pwm);
  
  // print the results to the serial monitor:
  //Serial.print("brightness : " );                      
  //Serial.println(brightnessValue);  
}

int nextBus(int hour, int minutes) {
  for (int index = 0; index < MAX_STOPS; index++)
  {
    if(timetable[hour][index] < minutes);
      //Serial.println(timetable[hour][index]);
    else {
      /*Serial.print("Prochain bus dans ");
      Serial.print(abs(minutes - timetable[hour][index]));
      Serial.println(" minutes.");*/
      return abs(minutes - timetable[hour][index]);
    }
  }
  /*Serial.print("Prochain bus dans ");
  Serial.print(timetable[hour+1][0] + (60 - minutes));
  Serial.println(" minutes.");*/
  return timetable[hour+1][0] + (60 - minutes);
}

void drawChart(byte x, byte pixel) {
  dotmatrix.rect(x, pixel, x+2, 15, GREEN);
  if(pixel < 14)
    dotmatrix.line(x+1, pixel+1, x+1, 14, BLACK);
  dotmatrix.line(x+3, pixel, x+3, 15, BLACK);
}

int stringToInt(String string) {
  char buffer[8];
  string.toCharArray(buffer, string.length()+1);
  
  return atoi(buffer);
}
