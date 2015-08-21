void connectToServer() {
  // attempt to connect, and wait a millisecond:
  #ifdef DEBUG
  Serial.print("Connecting to ");
  Serial.print(serverName);
  Serial.println("...");
  #endif
  if (client.connect(serverName, 80)) {
    #ifdef DEBUG
    Serial.println("Making HTTP request...");
    #endif
    // make HTTP GET request to twitter:
    client.print("GET /");
    client.print(apikey);
    client.println(".json HTTP/1.1");
    client.print("Host: ");
    client.println(serverName);
    client.println("User-Agent: Arduino/1.0");
    client.println("Connection: close");
    client.println();
  }
}

boolean parseJSON() {
  // make sure all apps are not enabled
  resetApps();
  String content = "";
  
  while(client.available()) {
    // read incoming bytes:
    char inChar = client.read();
    #ifdef WATCHDOG
    wdt_reset();
    #endif
    
    #ifdef DEBUG
    // debugging purposes only:
    //Serial.print(inChar);
    //Serial.println(currentLine);
    #endif
    
  
    // add incoming byte to end of line:
    currentLine += inChar;
  
    // if you get a newline, clear the line:
    if (inChar == '\n') {
      currentLine = "";
    }
    
    // fetch Time app data
    if (currentLine.endsWith("\"time\":")) {
      readingTime = true;
      content = "";
      currentLine = "";
    }
  
    if (readingTime) {
      if (inChar != ',' && inChar != '}') {
        if (inChar != '"' && inChar != ':')
          content += inChar;
      }
      else {
        readingTime = false;
        hour[0] = content.charAt(0);
        hour[1] = content.charAt(1);
        hour[3] = '\0';
        
        minutes[0] = content.charAt(2);
        minutes[1] = content.charAt(3);
        minutes[3] = '\0';
        
        timeEnabled = true;
    
        #ifdef DEBUG
        Serial.print("Time: ");
        Serial.print(hour);
        Serial.print(":");
        Serial.println(minutes);
        #endif
      }
    }
    
    // fetch dotmatrix speed
    if (currentLine.endsWith("\"speed\":")) {
      readingSpeed = true;
      content = "";
      currentLine = "";
    }
    
    if (readingSpeed) {
      if (inChar != ',' && inChar != '}') {
        if (inChar != '"' && inChar != ':')
          content += inChar;
      }
      else {
        readingSpeed = false;
        speed = stringToInt(content);
        
        #ifdef DEBUG
        Serial.print("Scrolling speed: ");
        Serial.println(speed);
        #endif
      }
    }
    
    // fetch dotmatrix sleeping mode
    if (currentLine.endsWith("\"sleeping\":")) {
      readingSleepingMode = true;
      content = "";
      currentLine = "";
    }
  
    if (readingSleepingMode) {
      if (inChar != ',' && inChar != '}') {
        if (inChar != '"' && inChar != ':')
        content += inChar;
      }
      else {
        readingSleepingMode = false;
        sleeping = content.charAt(0) == 't';
        
        #ifdef DEBUG
        Serial.print("Sleeping: ");
        Serial.println(sleeping);
        #endif
      }
    }
    
    // fetch Bus app data
    if (currentLine.endsWith("\"bus\":")) {
      readingBus = true;
      content = "";
      currentLine = "";
    }
  
    if (readingBus) {
       if (inChar != ',' && inChar != '}') {
        if (inChar != '"' && inChar != ':')
        content += inChar;
      }
      else {
        readingBus = false;
        bus[0] = content.charAt(0);
        bus[1] = content.charAt(1);
        bus[2] = '\0';
        
        busEnabled = true;
        
        #ifdef DEBUG
        Serial.print("Bus: ");
        Serial.println(bus);
        #endif
      }
    }
    
    #ifdef BUSSTOP
    // fetch Bus Stop app data
    if (currentLine.endsWith("\"route0\":")) {
      readingRoute0 = true;
      content = "";
      currentLine = "";
    }
  
    if (readingRoute0) {
       if (inChar != ',' && inChar != '}') {
        if (inChar != '"' && inChar != ':')
          content += inChar;
      }  
      else {
        readingRoute0 = false;
        route0[0] = content.charAt(0);
        route0[1] = content.charAt(1);
        route0[2] = '\0';
        
        busStopEnabled = true;
        
        #ifdef DEBUG
        Serial.print("Bus Stop: ");
        Serial.print(route0);
        #endif
      }
    }
    
    /*if (currentLine.endsWith("\"headsign0\":")) {
      readingHeadsign0 = true;
      content = "";
      currentLine = "";
    }
  
    if (readingHeadsign0) {
       if (inChar != ',' && inChar != '}') {
        if (inChar != '"' && inChar != ':')
          content += inChar;
      }
      else {
        readingHeadsign0 = false;
        content.toCharArray(headsign0, min(content.length() + 1, 32));
        headsign0[31] = '\0';
        
        #ifdef DEBUG
        Serial.print("-");
        Serial.print(headsign0);
        #endif
      }
    }*/
    
    if (currentLine.endsWith("\"departure0\":")) {
      readingDeparture0 = true;
      content = "";
      currentLine = "";
    }
  
    if (readingDeparture0) {
       if (inChar != ',' && inChar != '}') {
        if (inChar != '"' && inChar != ':')
        content += inChar;
      }  
      else {
        readingDeparture0 = false;
        departure0[0] = content.charAt(0);
        departure0[1] = content.charAt(1);
        departure0[2] = '\0';
        
        #ifdef DEBUG
        Serial.print(" in ");
        Serial.print(departure0);
        Serial.print("'");
        #endif
      }
    }
    
    if (currentLine.endsWith("\"route1\":")) {
      readingRoute1 = true;
      content = "";
      currentLine = "";
    }
  
    if (readingRoute1) {
       if (inChar != ',' && inChar != '}') {
        if (inChar != '"' && inChar != ':')
          content += inChar;
      }  
      else {
        readingRoute1 = false;
        route1[0] = content.charAt(0);
        route1[1] = content.charAt(1);
        route1[2] = '\0';
        
        #ifdef DEBUG
        Serial.print(",");
        Serial.print(route1);
        #endif
      }
    }
    
    /*if (currentLine.endsWith("\"headsign1\":")) {
      readingHeadsign1 = true;
      content = "";
      currentLine = "";
    }
  
    if (readingHeadsign1) {
       if (inChar != ',' && inChar != '}') {
        if (inChar != '"' && inChar != ':')
          content += inChar;
      }
      else {
        readingHeadsign1 = false;
        content.toCharArray(headsign1, min(content.length() + 1, 32));
        headsign1[31] = '\0';
        
        #ifdef DEBUG
        Serial.print("-");
        Serial.print(headsign1);
        #endif
      }
    }*/
    
    if (currentLine.endsWith("\"departure1\":")) {
      readingDeparture1 = true;
      content = "";
      currentLine = "";
    }
  
    if (readingDeparture1) {
       if (inChar != ',' && inChar != '}') {
        if (inChar != '"' && inChar != ':')
        content += inChar;
      }  
      else {
        readingDeparture1 = false;
        departure1[0] = content.charAt(0);
        departure1[1] = content.charAt(1);
        departure1[2] = '\0';
        
        #ifdef DEBUG
        Serial.print(" in ");
        Serial.print(departure1);
        Serial.println("'");
        #endif
      }
    }
    #endif
    
    // fetch Bikes app data
    if (currentLine.endsWith("\"bikes\":")) {
      readingBikes = true;
      content = "";
      currentLine = "";
    }
  
    if (readingBikes) {
      if (inChar != ',' && inChar != '}') {
        if (inChar != '"' && inChar != ':')
        content += inChar;
      }
      else {
        readingBikes = false;
        bikes[0] = content.charAt(0);
        bikes[1] = content.charAt(1);
        bikes[2] = '\0';
        
        bikesEnabled = true;
        
        #ifdef DEBUG
        Serial.print("Bikes: ");
        Serial.println(bikes);
        #endif
      }
    }
    
    #ifdef COFFEES
    // fetch Coffees app data
    if (currentLine.endsWith("\"coffees\":")) {
      readingCoffees = true;
      content = "";
      currentLine = "";
    }
  
    if (readingCoffees) {
       if (inChar != ',' && inChar != '}') {
        if (inChar != '"' && inChar != ':')
        content += inChar;
      }
      else {
        readingCoffees = false;
        coffees[0] = content.charAt(0);
        coffees[1] = content.charAt(1);
        coffees[2] = '\0';
        
        coffeesEnabled = true;
        
        #ifdef DEBUG
        Serial.print("Coffees: ");
        Serial.println(coffees);
        #endif
      }
    }
    #endif
    
    #ifdef EMAILS
    // fetch Emails app data
    if (currentLine.endsWith("\"emails\":")) {
      readingEmails = true;
      content = "";
      currentLine = "";
    }
  
    if (readingEmails) {
      if (inChar != ',' && inChar != '}') {
        if (inChar != '"' && inChar != ':')
        content += inChar;
      }
      else {
        readingEmails = false;
        emails[0] = content.charAt(0);
        emails[1] = content.charAt(1);
        emails[2] = '\0';
        
        emailsEnabled = true;
        
        #ifdef DEBUG
        Serial.print("Emails: ");
        Serial.println(emails);
        #endif
      }
    }
    #endif
    
    // fetch Energy app data
    #ifdef ENERGY
    if (currentLine.endsWith("\"day0\":")) {
      readingDay0 = true;
      content = "";
      currentLine = "";
    }
  
    if (readingDay0) {
       if (inChar != ',' && inChar != '}') {
        if (inChar != '"' && inChar != ':')
          content += inChar;
      }
      else {
        readingDay0 = false;
        energy[0] = stringToInt(content);
      }
    }
    
    if (currentLine.endsWith("\"day1\":")) {
      readingDay1 = true;
      content = "";
      currentLine = "";
    }
  
    if (readingDay1) {
       if (inChar != ',' && inChar != '}') {
        if (inChar != '"' && inChar != ':')
        content += inChar;
      }
      else {
        readingDay1 = false;
        energy[1] = stringToInt(content);
      }
    }
    
    if (currentLine.endsWith("\"day2\":")) {
      readingDay2 = true;
      content = "";
      currentLine = "";
    }
  
    if (readingDay2) {
       if (inChar != ',' && inChar != '}') {
        if (inChar != '"' && inChar != ':')
        content += inChar;
      }
      else {
        readingDay2 = false;
        energy[2] = stringToInt(content);
      }
    }
    
    if (currentLine.endsWith("\"day3\":")) {
      readingDay3 = true;
      content = "";
      currentLine = "";
    }
  
    if (readingDay3) {
       if (inChar != ',' && inChar != '}') {
        if (inChar != '"' && inChar != ':')
        content += inChar;
      }
      else {
        readingDay3 = false;
        energy[3] = stringToInt(content);
      }
    }
    
    if (currentLine.endsWith("\"day4\":")) {
      readingDay4 = true;
      content = "";
      currentLine = "";
    }
  
    if (readingDay4) {
       if (inChar != ',' && inChar != '}') {
        if (inChar != '"' && inChar != ':')
        content += inChar;
      }
      else {
        readingDay4 = false;
        energy[4] = stringToInt(content);
      }
    }
    
    if (currentLine.endsWith("\"day5\":")) {
      readingDay5 = true;
      content = "";
      currentLine = "";
    }
  
    if (readingDay5) {
       if (inChar != ',' && inChar != '}') {
        if (inChar != '"' && inChar != ':')
        content += inChar;
      }
      else {
        readingDay5 = false;
        energy[5] = stringToInt(content);
      }
    }
    
    if (currentLine.endsWith("\"day6\":")) {
      readingDay6 = true;
      content = "";
      currentLine = "";
    }
  
    if (readingDay6) {
       if (inChar != ',' && inChar != '}') {
        if (inChar != '"' && inChar != ':')
        content += inChar;
      }
      else {
        readingDay6 = false;
        energy[6] = stringToInt(content);
        
        energyEnabled = true;
        
        #ifdef DEBUG
        Serial.print("Energy: ");
        for(int i = 0; i < 7; i++) {
          Serial.print(energy[i]);
          if(i != 6)
            Serial.print(", ");
        }
        Serial.println();
        #endif
      }
    }
    #endif
    
    #ifdef AGENDA
    // fetch Agenda app data
    if (currentLine.endsWith("\"dtstart\":")) {
      readingEventStart = true;
      content = "";
      currentLine = "";
    }
  
    if (readingEventStart) {
      if (inChar != ',' && inChar != '}') {
        if (inChar != '"' && inChar != ':')
        content += inChar;
      }
      else {
        readingEventStart = false;
        content.toCharArray(eventStart, 5);
        eventStart[4] = '\0';
        
        agendaEnabled = true;
        
        #ifdef DEBUG
        Serial.print("Event : ");
        Serial.print(eventStart[0]);
        Serial.print(eventStart[1]);
        Serial.print(":");
        Serial.print(eventStart[2]);
        Serial.print(eventStart[3]);
        Serial.print(", ");
        #endif
      }
    }
    
    if (currentLine.endsWith("\"summary\":")) {
      readingEventSummary = true;
      content = "";
      currentLine = "";
    }
  
    if (readingEventSummary) {
      if (inChar != ',' && inChar != '}') {
        if (inChar != '"' && inChar != ':')
        content += inChar;
      }
      else {
        readingEventSummary = false;
        content.toCharArray(eventSummary, min(content.length() + 1, 64));
        eventSummary[64] = '\0';
        
        agendaEnabled = true;
        
        #ifdef DEBUG
        Serial.println(eventSummary);
        #endif
      }
    }
    #endif
    
    #ifdef MESSAGES
    // fetch Messages app data
    if (currentLine.endsWith("\"messages\":")) {
      readingMessage = true;
      content = "";
      currentLine = "";
    }
  
    if (readingMessage) {
      if (inChar != ',' && inChar != '}') {
        if (inChar != '"' && inChar != ':')
        content += inChar;
      }
      else {
        readingMessage = false;
        content.toCharArray(message, min(content.length() + 1, 140));
        message[139] = '\0';
        
        messagesEnabled = true;
        
        #ifdef DEBUG
        Serial.print("Messages: ");
        Serial.println(message);
        #endif
      }
    }
    #endif
    
    #ifdef PARKING
    // fetch Parking app data
    if (currentLine.endsWith("\"spaces\":")) {
      readingParkingSpaces = true;
      content = "";
      currentLine = "";
    }
    
    if (readingParkingSpaces) {
      if (inChar != ',' && inChar != '}') {
        if (inChar != '"' && inChar != ':')
          content += inChar;
      }
      else {
        readingParkingSpaces = false;
        parkingSpaces[0] = content.charAt(0);
        parkingSpaces[1] = content.charAt(1);
        parkingSpaces[2] = content.charAt(2);
        parkingSpaces[3] = '\0';
      }
    }
    
    if (currentLine.endsWith("\"open\":")) {
      readingParkingOpen = true;
      content = "";
      currentLine = "";
    }
  
    if (readingParkingOpen) {
      if (inChar != ',' && inChar != '}') {
        if (inChar != '"' && inChar != ':')
        content += inChar;
      }
      else {
        readingParkingOpen = false;
        parkingOpen = content.charAt(0) == 't';
        parkingEnabled = true;
        #ifdef DEBUG
        Serial.print("Parking: ");
        Serial.print(parkingOpen);
        Serial.print(", ");
        Serial.println(parkingSpaces);
        #endif
      }
    }
    #endif
    
    #ifdef METRO
    // fetch Metro app data
    if (currentLine.endsWith("\"failure\":")) {
      readingMetroFailure = true;
      content = "";
      currentLine = "";
    }
  
    if (readingMetroFailure) {
      if (inChar != ',' && inChar != '}') {
        if (inChar != '"' && inChar != ':')
        content += inChar;
      }
      else {
        readingMetroFailure = false;
        metroFailure[0] = content.charAt(0);
        metroFailure[1] = '\0';
        metroEnabled = true;
        #ifdef DEBUG
        Serial.print("Metro failure: ");
        Serial.println(metroFailure[0] == 't');
        #endif
      }
    }
    #endif
    
    // fetch Weather app data
    if (currentLine.endsWith("y\":{\"icon\":")) {
      readingTodayIcon = true;
      content = "";
      currentLine = "";
    }
  
    if (readingTodayIcon) {
      if (inChar != ',' && inChar != '}') {
        if (inChar != '"' && inChar != ':')
        content += inChar;
      }
      else {
        readingTodayIcon = false;
        todayIcon = stringToInt(content);
      }
    }
    
    if (currentLine.endsWith("\"temperature\":")) {
      readingTemperature = true;
      content = "";
      currentLine = "";
    }
  
    if (readingTemperature) {
      if (inChar != ',' && inChar != '}') {
        if (inChar != '"' && inChar != ':')
        content += inChar;
      }
      else {
        readingTemperature = false;
        temperature[0] = content.charAt(0);
        temperature[1] = content.charAt(1);
        temperature[2] = '\0';
      }
    }
    
    if (currentLine.endsWith("w\":{\"icon\":")) {
      readingTomorrowIcon = true;
      content = "";
      currentLine = "";
    }
  
    if (readingTomorrowIcon) {
      if (inChar != ',' && inChar != '}') {
        if (inChar != '"' && inChar != ':')
        content += inChar;
      }
      else {
        readingTomorrowIcon = false;
        tomorrowIcon = stringToInt(content);
      }
    }
    
    if (currentLine.endsWith("\"low\":")) {
      readingLow = true;
      content = "";
      currentLine = "";
    }
  
    if (readingLow) {
      if (inChar != ',' && inChar != '}') {
        if (inChar != '"' && inChar != ':')
        content += inChar;
      }
      else {
        readingLow = false;
        low[0] = content.charAt(0);
        low[1] = content.charAt(1);
        low[2] = '\0';
      }
    }
    
    if (currentLine.endsWith("\"high\":")) {
      readingHigh = true;
      content = "";
      currentLine = "";
    }
  
    if (readingHigh) {
      if (inChar != ',' && inChar != '}') {
        if (inChar != '"' && inChar != ':')
        content += inChar;
      }
      else {
        readingHigh = false;
        high[0] = content.charAt(0);
        high[1] = content.charAt(1);
        high[2] = '\0';
        
        weatherEnabled = true;
        
        #ifdef DEBUG
        Serial.print("Weather: ");
        Serial.print(todayIcon);
        Serial.print(", ");
        Serial.print(temperature);
        Serial.print(", ");
        Serial.print(tomorrowIcon);
        Serial.print(", ");
        Serial.print(low);
        Serial.print(", ");
        Serial.println(high);
        #endif
      }
    }
  }
}

void resetApps() {
  timeEnabled = false;
  weatherEnabled = false;
  busEnabled = false;
  bikesEnabled = false;
  #ifdef ENERGY
  energyEnabled = false;
  #endif
  #ifdef MESSAGES
  messagesEnabled = false;
  #endif
  #ifdef COFFEES
  coffeesEnabled = false;
  #endif
  #ifdef EMAILS
  emailsEnabled = false;
  #endif
  #ifdef AGENDA
  agendaEnabled = false;
  #endif
  #ifdef PARKING
  parkingEnabled = false;
  #endif
  #ifdef METRO
  metroEnabled = false;
  #endif
  #ifdef BUSSTOP
  busStopEnabled = false;
  #endif
}

int stringToInt(String string) {
  char buffer[8];
  string.toCharArray(buffer, string.length()+1);
  return atoi(buffer);
}

// dotmatrix functions
#ifdef HT1632C
void printTime(int x) {
  dotmatrix.putchar(x+5, 0, hour[0], GREEN);
  dotmatrix.putchar(x+10, 0, hour[1], GREEN);
  dotmatrix.putchar(x+14, 0, ':', GREEN);
  dotmatrix.putchar(x+18, 0, minutes[0], GREEN);
  dotmatrix.putchar(x+23, 0, minutes[1], GREEN);
}

void adjustBrightness() {
  // reset the watchdog timer
  #ifdef WATCHDOG
  wdt_reset();
  #endif
  
  // read the analog in value:
  #ifdef SENSORS
  brightnessValue = (analogRead(ldrPin) + previousBrightnessValue) / 2;
  pwm = (brightnessValue*15)/1023;
  dotmatrix.pwm(pwm);
  
  previousBrightnessValue = brightnessValue;
  #endif
  
}

void blinkPixel() {
  dotmatrix.clear();
  dotmatrix.plot(0, 0, GREEN);
  dotmatrix.sendframe();
  delay(500);
  dotmatrix.clear();
  delay(9000);
}

#ifdef BUSSTOP
void printBusStop(int x, char *departure, char *route) {
  if(departure[1] == '\0') {
    dotmatrix.putchar(x+1, 10,' ', GREEN);
    dotmatrix.putchar(x+3, 10, departure[0], GREEN);
    dotmatrix.putchar(x+8, 10, '\'', GREEN);
  }
  else {
    dotmatrix.putchar(x, 10, departure[0], GREEN);
    dotmatrix.putchar(x+5, 10, departure[1], GREEN);
    dotmatrix.putchar(x+10, 10, '\'', GREEN);
  }
  dotmatrix.putchar(x, 0, ' ', BLACK, 0, GREEN);
  dotmatrix.putchar(x, 1, ' ', BLACK, 0, GREEN);
  
  if(route[1] == '\0')
    dotmatrix.putchar(x+4, 1, route[0], BLACK, 0, GREEN);
  else {
    dotmatrix.putchar(x+6, 0, ' ', BLACK, 0, GREEN);
    dotmatrix.putchar(x+6, 1, ' ', BLACK, 0, GREEN);
    dotmatrix.putchar(x, 0, ' ', BLACK, 0, GREEN);
    dotmatrix.putchar(x, 1, ' ', BLACK, 0, GREEN);
    dotmatrix.putchar(x+1, 1, route[0], BLACK, 0, GREEN);
    dotmatrix.putchar(x+6, 1, route[1], BLACK, 0, GREEN);
  }
}
#endif
    

void waitAWhile() {
  for (int i = 0; i < 8; i++) {
    adjustBrightness();
    delay(30);
  }
}
void printTemperature(int x, char firstDigit, char secondDigit, byte color)
{
  if(secondDigit == '\0') {
    secondDigit = firstDigit;
    firstDigit = ' ';
  }
  dotmatrix.putchar(x, 10, firstDigit, color);
  dotmatrix.putchar(x+5, 10, secondDigit, color);
  dotmatrix.putchar(x+10, 10, '*', color);
}

void scrollFirstPanel(int x) {
  if(weatherEnabled) {
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
      #ifdef SENSORS
      waitAWhile();
      printTemperature(x+17, indoorTemperatureString[0], indoorTemperatureString[1], ORANGE);
      dotmatrix.sendframe();
      #endif
      dotmatrix.sendframe();
      waitAWhile();
    }
  }
  else
    x=-32;
}

void scrollSecondPanel(int x) {
  if(weatherEnabled) {
    // second panel : tomorrow weather condition 0→-32
    if(x <= 1 && x >= -48) {
      dotmatrix.putchar(x+44, 9, ' ', RED);
      color = tomorrowIcon == 0 ? color = ORANGE : color = RED;
      dotmatrix.putbitmap(x+32, 7, sprites[tomorrowIcon],16,9, color);
    }
    
    if(x >= -32 && x < 0) {
      #ifdef SENSORS
      printTemperature(x+17, indoorTemperatureString[0], indoorTemperatureString[1], ORANGE);
      #else
      printTemperature(x+17, temperature[0], temperature[1], RED);
      #endif
       
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
}

void scrollThirdPanel(int x) {
  //third panel : bus and bikes -32→-64
  if(x >= -63 && x < -32) {
    if(weatherEnabled)
      printTemperature(x+49, high[0], high[1], GREEN);
    if(timeEnabled)
      printTime(x+32);
  }
}


void scrollFourthPanel(int x) {
  if(x <= -65) {
    if(x == -65) {
      waitAWhile();      
      #ifdef BUSSTOP
      if(busStopEnabled) {
        waitAWhile();        
        printBusStop(x+67, departure1, route1);
        dotmatrix.sendframe();
        waitAWhile();
      }
      #endif
      waitAWhile();
    }
    
    if(x == -95) {
      waitAWhile();
      waitAWhile();
    }
    
    if(timeEnabled)
      printTime(x+129);
      
    #ifdef EMAILS
    // emails app
    if(emailsEnabled) {
      if(emails[1] != '\0')
        dotmatrix.putchar(x+116, 1, emails[1], GREEN);
      dotmatrix.putbitmap(x+100, 1, emailSprite, 9, 6, ORANGE);
      dotmatrix.putchar(x+109, 1, ' ', GREEN);
      dotmatrix.putchar(x+111, 1, emails[0], GREEN);
    }
    #endif
    
    #ifdef COFFEES
    // coffees app
    if(coffeesEnabled) {
      if(coffees[1] != '\0')
        dotmatrix.putchar(x+116, 2, coffees[1], GREEN);
      dotmatrix.putbitmap(x+99, 0, coffeeSprite, 16, 8, ORANGE);
      dotmatrix.putchar(x+108, 2, ' ', GREEN);
      dotmatrix.putchar(x+111, 2, coffees[0], GREEN);
    }
    #endif
    
    // parking app
    #ifdef PARKING
    if(parkingEnabled) {
      byte marginLeft = 0;
      // if we have two digits
      if(parkingSpaces[2] == '\0')
        marginLeft = 2;
      else
        marginLeft = 0;
        
      if(!marginLeft) {
        color = GREEN;
        dotmatrix.putchar(x+105+marginLeft, 10, parkingSpaces[2], color);
      }
      else {
        color = ORANGE;
        if(parkingSpaces[1] == '\0')
          color = RED;
      }
      
      dotmatrix.putchar(x+95+marginLeft, 10, parkingSpaces[0], color);
      if(parkingSpaces[1] == '\0')
        parkingSpaces[1] = '!';
      else
        dotmatrix.putchar(x+100+marginLeft, 10, parkingSpaces[1], color);
      
      if(!parkingOpen)
        color = RED;
        
      dotmatrix.putbitmap(x+97, 0, parkingSprite, 10, 8, color);
    }
    #endif
    
    // parking app
    #ifdef METRO
    if(metroEnabled) {
      if(metroFailure[0] == 't') {
        color = RED;
        dotmatrix.putchar(x+113, 10, '1', color);
        dotmatrix.putchar(x+118, 10, '1', color);
        dotmatrix.putchar(x+123, 10, '\'', color);
      }
      else {
        color = GREEN;
        dotmatrix.putchar(x+113, 10, 'O', color);
        dotmatrix.putchar(x+118, 10, 'K', color);
      }
      
      dotmatrix.putbitmap(x+113, 0, metroSprite, 11, 8, color);
    }
    #endif
    
    // energy app
    #ifdef ENERGY
    if(energyEnabled) {
      for(int i = 0; i < 7; i++) {
        drawChart(x + 96 + (i*4), energy[i]);
      }
    }
    #endif
  }
  
  if(x <= -33) {
    // bus app
    if(busEnabled && !busStopEnabled) {
      if(bus[1] == '\0') {
        dotmatrix.putchar(x+68, 10, bus[0], GREEN);
        dotmatrix.putchar(x+73, 10, '\'', GREEN);
      }
      else {
        dotmatrix.putchar(x+66, 10, bus[0], GREEN);
        dotmatrix.putchar(x+71, 10, bus[1], GREEN);
        dotmatrix.putchar(x+76, 10, '\'', GREEN);
      }
      
      dotmatrix.putbitmap(x+67, 0, busSprite, 9, 9, ORANGE);
    }
    
    #ifdef BUSSTOP
    // bus stop app
    if(busStopEnabled) {
      if(x>-65)
        printBusStop(x+66, departure0, route0);
      else
        printBusStop(x+66, departure1, route1);
    }
    #endif
    
    // bikes app
    if(bikesEnabled) {
      dotmatrix.putchar(x+92, 0, ' ', ORANGE);
      dotmatrix.putchar(x+92, 3, ' ', ORANGE);
      dotmatrix.putbitmap(x+77, 0, bikeSprite, 16, 9, ORANGE);
      
      if(bikes[1] != '\0') {
        dotmatrix.putchar(x+82, 10, bikes[0], GREEN);
        dotmatrix.putchar(x+87, 10, bikes[1], GREEN);
      }
      else
        dotmatrix.putchar(x+85, 10, bikes[0], GREEN);
    }
  }
  
  if(x <= -33) {
    // bus app
    if(busEnabled) {
      if(bus[0] == '-')
        bus[0] = '<';
      if(bus[1] == '\0') {
        dotmatrix.putchar(x+68, 10, bus[0], GREEN);
        dotmatrix.putchar(x+73, 10, '\'', GREEN);
      }
      else {
        dotmatrix.putchar(x+66, 10, bus[0], GREEN);
        dotmatrix.putchar(x+71, 10, bus[1], GREEN);
        dotmatrix.putchar(x+76, 10, '\'', GREEN);
      }
      
      dotmatrix.putbitmap(x+67, 0, busSprite, 9, 9, ORANGE);
    }
    
    // bikes app
    if(bikesEnabled) {
      dotmatrix.putchar(x+92, 0, ' ', ORANGE);
      dotmatrix.putchar(x+92, 3, ' ', ORANGE);
      dotmatrix.putbitmap(x+77, 0, bikeSprite, 16, 9, ORANGE);
      
      if(bikes[1] != '\0') {
        dotmatrix.putchar(x+82, 10, bikes[0], GREEN);
        dotmatrix.putchar(x+87, 10, bikes[1], GREEN);
      }
      else
        dotmatrix.putchar(x+85, 10, bikes[0], GREEN);
    }
  }
}

#ifdef AGENDA
void scrollFifthPanel(int x) {
  //fourth panel : coffees and energy -64→-96
  if(x <= -63) {
    // agenda app
    if(agendaEnabled) {
      dotmatrix.putchar(x+133, 0, ' ', ORANGE);
      dotmatrix.putchar(x+138, 0, ' ', ORANGE);
      dotmatrix.putchar(x+142, 0, ' ', ORANGE);
      dotmatrix.putchar(x+147, 0, ' ', ORANGE);
      dotmatrix.putchar(x+152, 0, ' ', ORANGE);
      dotmatrix.putchar(x+157, 0, ' ', ORANGE);
      dotmatrix.putchar(x+133, 1, ' ', ORANGE);
      dotmatrix.putbitmap(x+129, 0, calendarSprite,8,8, RED);
      dotmatrix.putchar(x+138, 1, eventStart[0], ORANGE);
      dotmatrix.putchar(x+143, 1, eventStart[1], ORANGE);
      dotmatrix.putchar(x+147, 1, ':', ORANGE);
      dotmatrix.putchar(x+151, 1, eventStart[2], ORANGE);
      dotmatrix.putchar(x+156, 1, eventStart[3], ORANGE);
      
      if(x == -129)
        dotmatrix.hscrolltext(9, eventSummary, ORANGE, speed-20, 1, LEFT);
      if(timeEnabled)
        printTime(x+161);
    }
  }
}
#endif

#ifdef MESSAGES
void scrollSixthPanel() {
  // fifth panel : message
  if(messagesEnabled)
    dotmatrix.hscrolltext(9, message, GREEN, speed-20, 1, LEFT);
}
#endif

#ifdef ENERGY
void drawChart(byte x, byte height) {
  dotmatrix.rect(x, 16-height, x+2, 15, GREEN);
  if(height > 2)
    dotmatrix.line(x+1, 17-height, x+1, 14, BLACK);
  dotmatrix.line(x+3, 16-height, x+3, 15, BLACK);
}
#endif
#endif

#ifdef SENSORS
int getTemperature() {
  // smallest footprint for temperature reading http://playground.arduino.cc/ComponentLib/Thermistor3
  return ((analogRead(thermistorPin) - 250) * (441 - 14) / (700 - 250) + 250)/10;
  // please have a look at TinkerKit! library for more info : http://www.tinkerkit.com/library/
  /*
  // too heavy
  int Rthermistor = 10000 * (1023 / analogRead(thermistorPin) - 1);
  int temperatureC = 3950 / (log(Rthermistor * 120 )) ;
  
  return temperatureC - 273;*/
}
#endif
