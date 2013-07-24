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
  String content = "";
  
  while(client.available()) {
    // read incoming bytes:
    char inChar = client.read();
    wdt_reset();
    
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
    }
  
    if (readingTime) {
      if (inChar != ',' && inChar != '}') {
        if (inChar != '\"' && inChar != ':')
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
    
    // fetch Bus app data
    if (currentLine.endsWith("\"bus\":")) {
      readingBus = true;
      content = "";
    }
  
    if (readingBus) {
       if (inChar != ',' && inChar != '}') {
        if (inChar != '\"' && inChar != ':')
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
    
    if (currentLine.endsWith("\"bikes\":")) {
      readingBikes = true;
      content = "";
    }
  
    if (readingBikes) {
      if (inChar != ',' && inChar != '}') {
        if (inChar != '\"' && inChar != ':')
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
    
    if (currentLine.endsWith("\"emails\":")) {
      readingEmails = true;
      content = "";
    }
  
    if (readingEmails) {
      if (inChar != ',' && inChar != '}') {
        if (inChar != '\"' && inChar != ':')
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
    
    // fetch Weather app data
    if (currentLine.endsWith("\"icon\":")) {
      readingTodayIcon = true;
      content = "";
    }
  
    if (readingTodayIcon) {
      if (inChar != ',' && inChar != '}') {
        if (inChar != '\"' && inChar != ':')
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
    }
  
    if (readingTemperature) {
      if (inChar != ',' && inChar != '}') {
        if (inChar != '\"' && inChar != ':')
        content += inChar;
      }
      else {
        readingTemperature = false;
        temperature[0] = content.charAt(0);
        temperature[1] = content.charAt(1);
        temperature[2] = '\0';
      }
    }
    
    if (currentLine.endsWith("\"icon\":") && todayIconRead) {
      readingTomorrowIcon = true;
      content = "";
    }
  
    if (readingTomorrowIcon) {
      if (inChar != ',' && inChar != '}') {
        if (inChar != '\"' && inChar != ':')
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
    }
  
    if (readingLow) {
      if (inChar != ',' && inChar != '}') {
        if (inChar != '\"' && inChar != ':')
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
    }
  
    if (readingHigh) {
      if (inChar != ',' && inChar != '}') {
        if (inChar != '\"' && inChar != ':')
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

int stringToInt(String string) {
  char buffer[8];
  string.toCharArray(buffer, string.length()+1);
  return atoi(buffer);
}
