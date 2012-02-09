/*

 Weather Station v0.5
 
 Key Features:
 * Indoor Temperature
 * 2 days forecasts
 * Automatic screen brightness adjusting
 * Automatic time (NTP)
 * Weather forecast icons (sunny, cloudy, rain, snow, fog)
 
 Circuit:
 * Ethernet shield attached to pins 10, 11, 12, 13
 * Sure Electronics 3216 LED matrix attached to pins 4, 5, 6, 7
 * TinkerKit LDR, Thermistor and Touch modules on I0, I1, I2
 
 created 15 Dec 2011
 by Baptiste Gaultier and Tanguy Ropitault
 
 */
 
 
#include <SPI.h>
#include <Ethernet.h>
#include <ht1632c.h>
#include <TinkerKit.h>

// initialize the dotmatrix with the numbers of the interface pins
ht1632c dotmatrix = ht1632c(PORTD, 7, 6, 4, 5, GEOM_32x16, 2);


TKLightSensor ldr(I0);    // creating the object 'ldr' that belongs to the 'TKLightSensor' class

/*TKThermistor therm(I1);   // creating the object 'therm' that belongs to the 'TKThermistor' class 

TKTouchSensor touch(I2);  // creating the object 'touch' that belongs to the 'TKTouchSensor' class*/

int brightnessValue = 0; // value read from the LDR
byte pwm = 6;            // value output to the PWM (analog out)

byte todayIcon;
byte tomorrowIcon;
byte color;
char indoorTemperatureString[3] = "22";
float indoorTemperature;            // temperature readings are returned in float format

// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:
byte mac[] = { 0x90, 0xA2, 0xDA, 0x00, 0x65, 0xA4 };

// fill in an available IP address on your network here,
// for auto-configuration:
IPAddress ip(169, 254, 0, 64);
IPAddress subnet(255, 255, 0, 0);

// initialize the library instance:
EthernetClient client;

const int requestInterval = 10000;  // delay between requests

char serverName[] = "api.baptistegaultier.fr"; // Your favorite weather server

boolean requested;                   // whether you've made a request since connecting
long lastAttemptTime = 0;            // last time you connected to the server, in milliseconds

String currentLine = "";             // string to hold the text from server
String data = "";                    // string to hold the data
boolean reading = false;             // if you're currently reading the data

// weather forecast sprites:
uint16_t sprites[6][9] =
{
  { 0x0100, 0x0100, 0x2008, 0x1390, 0x0440, 0x0820, 0x682c, 0x0820, 0x0440 },
  { 0x0000, 0x01c0, 0x0230, 0x1c08, 0x2208, 0x4004, 0x4004, 0x3ff8, 0x0000 },
  { 0x01c0, 0x0230, 0x1c08, 0x2208, 0x4004, 0x4004, 0x3ff8, 0x1500, 0x1500 },
  { 0x0000, 0x0000, 0x7ffe, 0x0000, 0x7ffe, 0x0000, 0x7ffe, 0x0000, 0x0000 },
  { 0x0540, 0x0380, 0x1110, 0x0920, 0x1ff0, 0x0920, 0x1110, 0x0380, 0x0540 }
};


void setup() {
  // reserve space for the strings:
  currentLine.reserve(64);
  data.reserve(16);
  
  // initialize serial:
  Serial.begin(9600);
  // initialize dotmatrix:
  dotmatrix.clear();
  dotmatrix.pwm(pwm);
  
  // display a welcome message:
  Serial.println("Weather Station v0.5 starting...");
  
  // attempt a DHCP connection:
  if (!Ethernet.begin(mac)) {
    // if DHCP fails, start with a hard-coded address:
    Ethernet.begin(mac, ip, subnet);
  }
  
  // print your local IP address:
  Serial.print("My IP address: ");
  ip = Ethernet.localIP();
  for (byte thisByte = 0; thisByte < 4; thisByte++)
  {
    // print the value of each byte of the IP address:
    Serial.print(ip[thisByte], DEC);
    Serial.print("."); 
  }
  Serial.println();
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
      // if the current line ends with <text>, it will
      // be followed by the data:
      if ( currentLine.endsWith("data:")) {
        // data is beginning. Clear the data string:
        reading = true; 
        data = "";
      }
      // if you're currently reading the bytes of a data,
      // add them to the data String:
      if (reading) {
        if (inChar != ';') {
          data += inChar;
        } 
        else {
          // if you got a "<" character,
          // you've reached the end of the line:
          reading = false;
          
          dotmatrix.setfont(FONT_5x7);
          
          dotmatrix.putchar(5, 0, data.charAt(1), GREEN);
          dotmatrix.putchar(10, 0, data.charAt(2), GREEN);
          dotmatrix.putchar(14, 0, ':', GREEN);
          dotmatrix.putchar(18, 0, data.charAt(4), GREEN);
          dotmatrix.putchar(23, 0, data.charAt(5), GREEN);
          
          dotmatrix.sendframe();
          
          indoorTemperatureString[0] = data.charAt(7);
          indoorTemperatureString[1] = '\0';
          todayIcon = atoi(indoorTemperatureString);
          
          indoorTemperatureString[0] = data.charAt(12);
          indoorTemperatureString[1] = '\0';
          tomorrowIcon = atoi(indoorTemperatureString);
          
          // Reading the temperature in Celsius degrees and store in the indoorTemperature variable
          //indoorTemperature = therm.getCelsius();
          
          //dtostrf(indoorTemperature, 2, 0, indoorTemperatureString);
          
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
              printTemperature(x+17, data.charAt(9), data.charAt(10), RED);
              dotmatrix.sendframe();
            }
            
            if(x >= -32 && x < 0)
            {
              printTemperature(x+17, indoorTemperatureString[0], indoorTemperatureString[1], GREEN);
              printTemperature(x+49, data.charAt(14), data.charAt(15), RED);
              dotmatrix.sendframe();
            }
            
            if(x >= -63 && x < -32) {
              printTemperature(x+49, data.charAt(17), data.charAt(18), GREEN);
              dotmatrix.sendframe();
            }
            
            
            delay(50);
            
            if(x == 0)
            {
              delay(800);
              printTemperature(x+17, indoorTemperatureString[0], indoorTemperatureString[1], GREEN);
              dotmatrix.sendframe();
              delay(800);
            }
            
            if(x == -32)
            {
              delay(800);
              printTemperature(x+49, data.charAt(17), data.charAt(18), GREEN);
              dotmatrix.sendframe();
              delay(800);
            }
          }
          
          // close the connection to the server:
          client.stop(); 
        }
      }
    }   
  }
  else if (millis() - lastAttemptTime > requestInterval) {
    // if you're not connected, and two minutes have passed since
    // your last connection, then attempt to connect again:
    connectToServer();
  }
}

void connectToServer()
{
  // attempt to connect:
  Serial.print("Connecting to ");
  Serial.print(serverName);
  Serial.println("...");
  if (client.connect(serverName, 80))
  {
    Serial.println("Making HTTP request...");
    // make HTTP GET request to API server:
    client.println("GET /weather.php HTTP/1.1");
    client.println("Host: api.baptistegaultier.fr");
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
  dotmatrix.putchar(x+10, 9, '\'', color);
  
  // read the analog in value:
  brightnessValue = ldr.get();
  pwm = map(brightnessValue, 0, 1023, 0, 15);
  dotmatrix.pwm(pwm);
  
  // print the results to the serial monitor:
  Serial.print("brightness : " );                      
  Serial.println(brightnessValue);  
}
