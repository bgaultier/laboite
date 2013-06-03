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
