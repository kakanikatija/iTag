// Menu for when iTag connected via Serial Interface
#define PACKET 1024

void setupMenu(){
//  displayMenu();

  while(1){
    pinMode(ledGreen, OUTPUT);
    digitalWrite(ledGreen,LED_ON);
    if(SerialUSB.available()){
      int inByte = SerialUSB.read();
   //   SerialUSB.print("Selection:"); SerialUSB.println(inByte);
      switch (inByte){
        case 49:  //list files
          listFiles();
          clearSerial();
          break;
        case 50: // download files
          downloadFiles();
          clearSerial();
          break;
        case 51: // set time
          serialSetTime();
          clearSerial();
          break;
        case 52: // sensor Test
          sensorInit();
          clearSerial();
          break;
        case 53: // delete Files
          deleteFiles();
          clearSerial();
          break;
        case 54: // start
          SerialUSB.println("Starting");
          getTime();
          if(burnFlag==2){
            long curTime = RTCToUNIXTime(year, month, day, hour, minute, second);
            if(curTime > burnTime){
              for(int i = 0; i<10; i++){
              SerialUSB.println("Burn time has already passed");
              delay(1000);
              SerialUSB.println();
              }
            }
          }
          if(burnFlag==1){
            long curTime = RTCToUNIXTime(year, month, day, hour, minute, second);
            burnTime = curTime + (burnDelayMinutes * 60);
          }
          clearSerial();
          return;
        case 55: // set burn minutes
          setBurnMinutes();
          break;
        case 56: // set burn minutes
          setBurnTime();
          break;
          
      }
      inByte = 0;
   //   displayMenu();
    }
  }
}

void displayMenu(){
  SerialUSB.println();
  SerialUSB.print("iTag: ");
  printChipId();
  getTime();
  printTime();
  SerialUSB.println("(1) List files");
  SerialUSB.println("(2) Download File");
  SerialUSB.println("(3) Set time");
  SerialUSB.println("(4) Sensor test");
  SerialUSB.println("(5) Delete all files");
  SerialUSB.println("(6) Start");
  SerialUSB.println("(7) Set Burn Minutes");
  SerialUSB.println("(8) Set Burn Time");
}

void printChipId() {
  volatile uint32_t val1, val2, val3, val4;
  volatile uint32_t *ptr1 = (volatile uint32_t *)0x0080A00C;
  val1 = *ptr1;
  volatile uint32_t *ptr = (volatile uint32_t *)0x0080A040;
  val2 = *ptr;
  ptr++;
  val3 = *ptr;
  ptr++;
  val4 = *ptr;
  char buf[42];
  sprintf(buf, "iTag %8x%8x%8x%8x", val1, val2, val3, val4);
  SerialUSB.println(buf);
}

void listFiles(){
  File root;
  root = SD.open("/");
  printChipId();
  printDirectory(root);
}

void deleteFiles(){
  SerialUSB.println("Press Delete All again to delete. Any other button to cancel.");
  while(1){
    if(SerialUSB.available()){
      char inChar = SerialUSB.read();
      if(inChar != '5') {
        SerialUSB.println("Canceling delete");
        return;
      }
      else break;
    }
  }
  SerialUSB.println("Deleting Files");
  File dir;
  char filename[40];
  dir = SD.open("/");
  while (true) {
    File entry =  dir.openNextFile();
    if (! entry) {
      // no more files
      break;
    }
    strcpy(filename, entry.name());
    if (entry.isDirectory()) {
      entry.close();
    } else {
      // delete file
      entry.close();
      SerialUSB.print("Delete ");
      SerialUSB.print(filename); SerialUSB.print(" ");
      SerialUSB.println(SD.remove(filename));
    }
  }
  SerialUSB.println("Done deleting");
}
void serialSetTime(){
  // YYMMDDHHMMSS
  char inputStr[2];
//  SerialUSB.println("Enter Datetime (YYMMDDHHMMSS)");
//  SerialUSB.flush();
//  clearSerial();
  char datetime[12];
  int i = 0;
  long startTime = millis();
   while(1){
    if(SerialUSB.available()){
      char inChar = SerialUSB.read();
      datetime[i] = inChar;
      i++;
      if(i>=12) break;
      if(millis()-startTime > 10000) {
        SerialUSB.println("Set time fail");
        SerialUSB.flush(); // clear buffer
        return; // timeout
      }
    }
  }
  inputStr[0] = datetime[0]; inputStr[1] = datetime[1]; year = atoi(inputStr);
  inputStr[0] = datetime[2]; inputStr[1] = datetime[3]; month = atoi(inputStr);
  inputStr[0] = datetime[4]; inputStr[1] = datetime[5]; day = atoi(inputStr);
  inputStr[0] = datetime[6]; inputStr[1] = datetime[7]; hour = atoi(inputStr);
  inputStr[0] = datetime[8]; inputStr[1] = datetime[9]; minute = atoi(inputStr);
  inputStr[0] = datetime[10]; inputStr[1] = datetime[11]; second = atoi(inputStr);
  rtc.setTime(hour, minute, second);
  rtc.setDate(day, month, year);
  SerialUSB.println("New Time: "); printTime();
}
//170202142700


void printDirectory(File dir) {
  while (true) {
    File entry =  dir.openNextFile();
    if (! entry) {
      // no more files
      break;
    }
    if (entry.isDirectory()) {
      //SerialUSB.println("/");
      //printDirectory(entry, numTabs + 1);
    } else {
      // files have sizes, directories do not
      SerialUSB.print(entry.name());
      SerialUSB.print(',');
      SerialUSB.println(entry.size(), DEC);
    }
    entry.close();
  }
}

void printTime(){
  getTime();
  SerialUSB.print(year); SerialUSB.print("-");
  SerialUSB.print(month); SerialUSB.print("-");
  SerialUSB.print(day); SerialUSB.print(" ");
  SerialUSB.print(hour); SerialUSB.print(":");
  SerialUSB.print(minute); SerialUSB.print(":");
  SerialUSB.println(second); 
}

void clearSerial(){
  while(SerialUSB.available()){
      int inByte = SerialUSB.read();
  }
}


void downloadFiles(){
//  SerialUSB.println("Enter Filename ");
//  SerialUSB.flush();
  int inByte = 100;
  char filename[15];
  memset(filename, 0, 15);
  int index = 0;
  long startTime = millis();
  while(inByte > 30){  //wait for carriage return
    if(SerialUSB.available()){
      inByte =  SerialUSB.read();
      filename[index] = inByte;
      index++;
      if(inByte == 'V' | inByte == 'X') break;
    }
    if(millis() - startTime > 5000) break;
  }
  clearSerial();

  File file;
  if(file = SD.open(filename)){
    sendFile(&file);
    file.close();  //just do one file for now
  }
  else{
    SerialUSB.println("File open fail");
  }
}

void sendFile(File *file){
  // send data 1024 bytes at a time
  
  byte data[PACKET];
  int bytes2write;
  while(file->available()){
    long bytesAvail = file->available();
    file->read(data, PACKET);
    bytes2write = PACKET;
    if(bytesAvail < PACKET) bytes2write = bytesAvail;
    SerialUSB.write(data, bytes2write);
  }
}

void setBurnMinutes(){
  int inByte = 100;
  char burnMin[15];
  memset(burnMin, 0, 15);
  int index = 0;
  long startTime = millis();
  while(inByte > 30){  //wait for carriage return
    if(SerialUSB.available()){
      inByte =  SerialUSB.read();
      burnMin[index] = inByte;
      index++;
      if(inByte == 'X') break;
    }
    if(millis() - startTime > 5000) break;
  }
  clearSerial();

  burnDelayMinutes = atoi(burnMin);
  if(burnDelayMinutes < 0) burnDelayMinutes = 0;

  if(burnDelayMinutes == 0) {
    burnFlag = 0;
    SerialUSB.println("Burn wire disabled");
  }
  else{
    SerialUSB.print("Burn delay: "); SerialUSB.println(burnDelayMinutes);
    long curTime = RTCToUNIXTime(year, month, day, hour, minute, second);
    burnTime = curTime + (burnDelayMinutes * 60);
    SerialUSB.print("Now:"); SerialUSB.println(curTime);
    SerialUSB.print("Burn:"); SerialUSB.println(burnTime);
    burnFlag = 1;
  }

}

void setBurnTime(){
  char inputStr[2];
  char datetime[12];
  int i = 0;
  long startTime = millis();
   while(1){
    if(SerialUSB.available()){
      char inChar = SerialUSB.read();
      datetime[i] = inChar;
      i++;
      if(i>=12) break;
      if(millis()-startTime > 10000) {
        SerialUSB.println("Set burn time fail");
        SerialUSB.flush(); // clear buffer
        return; // timeout
      }
    }
  }
  inputStr[0] = datetime[0]; inputStr[1] = datetime[1]; burnYear = atoi(inputStr);
  inputStr[0] = datetime[2]; inputStr[1] = datetime[3]; burnMonth = atoi(inputStr);
  inputStr[0] = datetime[4]; inputStr[1] = datetime[5]; burnDay = atoi(inputStr);
  inputStr[0] = datetime[6]; inputStr[1] = datetime[7]; burnHour = atoi(inputStr);
  inputStr[0] = datetime[8]; inputStr[1] = datetime[9]; burnMinute = atoi(inputStr);
  inputStr[0] = datetime[10]; inputStr[1] = datetime[11]; burnSecond = atoi(inputStr);

  long curTime = RTCToUNIXTime(year, month, day, hour, minute, second);
  burnTime = RTCToUNIXTime(burnYear, burnMonth, burnDay, burnHour, burnMinute, burnSecond);
  SerialUSB.print("Now:"); SerialUSB.println(curTime);
  SerialUSB.print("Burn:"); SerialUSB.println(burnTime);
  burnFlag = 2;
}

