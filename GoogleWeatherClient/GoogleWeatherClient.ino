/*
  Google Wheater Client with Strings
 
 This sketch connects to Google using an Ethernet shield. It parses the XML
 returned, and looks for wheater conditions
 
 You can use the Arduino Ethernet shield, or the Adafruit Ethernet shield, 
 either one will work, as long as it's got a Wiznet Ethernet module on board.
 
 This example uses the DHCP routines in the Ethernet library which is part of the 
 Arduino core from version 1.0 beta 1
 
 This example uses the String library, which is part of the Arduino core from
 version 0019.  
 
 Circuit:
 * Ethernet shield attached to pins 10, 11, 12, 13
 
 based on Twitter Client example by Tom Igoe
 modified 21 April 2012
 by Baptiste Gaultier
 
 This code is in the public domain.
 
 */
 
#include <SPI.h>
#include <Ethernet.h>


// Enter a MAC address and IP address for your controller below.
byte mac[] = { 
  0x90, 0xA2, 0xDA, 0x00, 0x68, 0x60 };

// fill in an available IP address on your network here,
// for auto-configuration:
IPAddress ip(169, 254, 0, 64);
IPAddress subnet(255, 255, 0, 0);

// initialize the library instance:
EthernetClient client;

const int requestInterval = 5000;  // delay between requests

char serverName[] = "google.com";  // google URL

boolean requested;                   // whether you've made a request since connecting
long lastAttemptTime = 0;            // last time you connected to the server, in milliseconds

char buffer[24];                    // buffer used for type convertion
byte icon;                          // byte to hold the icon
byte humidity;                      // byte to hold the humidity
byte temperature;                   // byte to hold the temperature


String currentLine = "";            // string to hold the text from server
String humidityString = "";         // string to hold the humidity
String temperatureString = "";      // string to hold the temperature
String iconString = "";             // string to hold the icon
boolean readingHumidity = false;    // if you're currently reading the humidity
boolean readingTemperature = false; // if you're currently reading the temperature
boolean readingIcon = false;        // if you're currently reading the icon

prog_char weatherCondition_0[] PROGMEM = "/sunny.gif\"/";
prog_char weatherCondition_1[] PROGMEM = "/mostly_sunny.gif\"/";
prog_char weatherCondition_2[] PROGMEM = "/cloudy.gif\"/";
prog_char weatherCondition_3[] PROGMEM = "/partly_cloudy.gif\"/";
prog_char weatherCondition_4[] PROGMEM = "/mostly_cloudy.gif\"/";
prog_char weatherCondition_5[] PROGMEM = "/rain.gif\"/";
prog_char weatherCondition_6[] PROGMEM = "/chance_of_rain.gif\"/";
prog_char weatherCondition_7[] PROGMEM = "/storm.gif\"/";
prog_char weatherCondition_8[] PROGMEM = "/thunderstorm.gif\"/";
prog_char weatherCondition_9[] PROGMEM = "/chance_of_storm.gif\"/";
prog_char weatherCondition_10[] PROGMEM = "/dust.gif\"/";
prog_char weatherCondition_11[] PROGMEM = "/fog.gif\"/";
prog_char weatherCondition_12[] PROGMEM = "/smoke.gif\"/";
prog_char weatherCondition_13[] PROGMEM = "/haze.gif\"/";
prog_char weatherCondition_14[] PROGMEM = "/mist.gif\"/";
prog_char weatherCondition_15[] PROGMEM = "/flurries.gif\"/";
prog_char weatherCondition_16[] PROGMEM = "/snow.gif\"/";
prog_char weatherCondition_17[] PROGMEM = "/chance_of_snow.gif\"/";
prog_char weatherCondition_18[] PROGMEM = "/icy.gif\"/";
prog_char weatherCondition_19[] PROGMEM = "/sleet.gif\"/";

PROGMEM const char *weatherConditions[] =
{
  weatherCondition_0,
  weatherCondition_1,
  weatherCondition_2,
  weatherCondition_3,
  weatherCondition_4,
  weatherCondition_5,
  weatherCondition_6,
  weatherCondition_7,
  weatherCondition_8,
  weatherCondition_9,
  weatherCondition_10,
  weatherCondition_11,
  weatherCondition_12,
  weatherCondition_13,
  weatherCondition_14,
  weatherCondition_15,
  weatherCondition_16,
  weatherCondition_17,
  weatherCondition_18,
  weatherCondition_19
};

void setup() {
  // reserve space for the strings:
  currentLine.reserve(256);
  temperatureString.reserve(50);
  humidityString.reserve(50);
  iconString.reserve(50);

  // initialize serial:
  Serial.begin(9600);
  Serial.println("Google Weather client starting...");
  // attempt a DHCP connection:
  if (!Ethernet.begin(mac)) {
    // if DHCP fails, start with a hard-coded address:
    Ethernet.begin(mac, ip);
  }

  Serial.println("Google Weather client starting...");
  // connect to Twitter:
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
      // if the current line ends with <temperature_c, it will
      // be followed by the temperature:
      if ( currentLine.endsWith("<temp_c data=\"")) {
        // temperature is beginning. Clear the temperature string:
        readingTemperature = true; 
        temperatureString = "";
      }

      // if the current line ends with <humidity data, it will
      // be followed by the humidity:
      if ( currentLine.endsWith("<humidity data=\"Humidity: ")) {
        // humidity is beginning. Clear the humidity string:
        readingHumidity = true; 
        humidityString = "";
      }
      
      // if the current line ends with <icon data, it will
      // be followed by the icon:
      if ( currentLine.endsWith("<icon data=\"/ig/images/weather/")) {
        // humidity is beginning. Clear the humidity string:
        readingIcon = true; 
        iconString = "";
      }

      // if you're currently reading the bytes of a temperature,
      // add them to the temperature String:
      if (readingTemperature)
      {
        if (inChar != '>')
        {
          temperatureString += inChar;
        } 
        else {
          // if you got a ">" character,
          // you've reached the end of the temperature:
          readingTemperature = false;
        }
      }
      // if you're currently reading the bytes of a humidity,
      // add them to the humidity String:
      if (readingHumidity)
      {
        if (inChar != '>')
        {
          humidityString += inChar;
        } 
        else {
          // if you got a ">" character,
          // you've reached the end of the humidity:
          readingHumidity = false;
        }
      }
      
      // if you're currently reading the bytes of a icon,
      // add them to the icon String:
      if (readingIcon)
      {
        if (inChar != '>')
        {
          iconString += inChar;
        } 
        else {
          // if you got a ">" character,
          // you've reached the end of the icon:
          readingIcon = false;
          Serial.print("icon : ");
          icon = iconStringToIcon(iconString);
          Serial.println(icon);
          // close the connection to the server:
          client.stop();
          // data convertion
          buffer[0] = temperatureString.charAt(1);
          buffer[1] = temperatureString.charAt(2);
          buffer[3] = '\0';
          temperature = atoi(buffer);
          Serial.print("temperature : ");
          Serial.print(temperature);
          Serial.println(" degrees");
          
          buffer[0] = humidityString.charAt(1);
          buffer[1] = humidityString.charAt(2);
          buffer[3] = '\0';
          humidity = atoi(buffer);
          Serial.print("humidity : ");
          Serial.print(humidity);
          Serial.println("%");
          
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
  // attempt to connect, and wait a millisecond:
  Serial.println("connecting to server...");
  if (client.connect(serverName, 80)) {
    Serial.println("making HTTP request...");
    // make HTTP GET request to google:
    client.println("GET /ig/api?weather=Tel-Aviv HTTP/1.1");
    client.println();
  }
  // note the time of this connect attemperaturet:
  lastAttemptTime = millis();
}



byte iconStringToIcon(String iconString)
{
  for (int i = 0; i < 6; i++)
  {
    strcpy_P(buffer, (char*)pgm_read_word(&(weatherConditions[i]))); // Necessary casts and dereferencing, just copy. 
    if(iconString.equals(buffer))
    {
      if (i < 2)
        return 0;
      if(i >= 2 && i < 5)
        return 1;
      if(i >= 5 && i < 10)
        return 2;
      if(i >= 10 && i < 15)
        return 3;
      if(i >= 15)
        return 4;
    }
  }
}  
