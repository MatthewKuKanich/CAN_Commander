// Custom CAN bus Controller Tester by Matthew KuKanich
#include <SPI.h>
#include <mcp2515.h>

struct can_frame canMsg;
MCP2515 mcp2515(10); // CS pin 10

enum Mode {
  MODE_NONE,
  MODE_READ_ALL,
  MODE_READ_FILTERED,
  MODE_WRITE,
  MODE_SPEEDTEST
};

Mode currentMode = MODE_NONE;

void setup() {
  Serial.begin(115200);
  while (!Serial); // Wait for serial port to connect.
  
  mcp2515.reset();
  mcp2515.setBitrate(CAN_500KBPS); // Set to default HS CAN bus, use 125 for MS
  mcp2515.setNormalMode();
  
  Serial.println("Select mode:");
  Serial.println("1: CAN Read All Traffic");
  Serial.println("2: CAN Write");
  Serial.println("3: CAN Speedtest");
  Serial.println("4: CAN Read Filtered Traffic");

  while (currentMode == MODE_NONE) {
    if (Serial.available() > 0) {
      int modeSelect = Serial.parseInt();
      switch (modeSelect) {
        case 1:
          currentMode = MODE_READ_ALL;
          Serial.println("CAN Read All Traffic selected.");
          Serial.println("ID  DLC   DATA");
          break;
        case 2:
          currentMode = MODE_WRITE;
          Serial.println("Enter CAN message to send (ID DLC DATA...):");
          while (!Serial.available()) ; // Wait for user input
          prepareCanWrite();
          Serial.println("CAN Write selected.");
          break;
        case 3:
          currentMode = MODE_SPEEDTEST;
          Serial.println("CAN Speedtest selected.");
          break;
        case 4:
          currentMode = MODE_READ_FILTERED;
          Serial.println("Enter Mask and Filter (Mask Filter):");
          while (!Serial.available()) ; // Wait for user input
          setupCanFilters();
          Serial.println("CAN Read Filtered Traffic selected.");
          break;
        default:
          Serial.println("Invalid selection. Select 1, 2, 3, or 4.");
          break;
      }
    }
  }
}

void loop() {
  switch (currentMode) {
    case MODE_READ_ALL:
      canReadAll();
      break;
    case MODE_WRITE:
      canWrite();
      break;
    case MODE_SPEEDTEST:
      canSpeedtest();
      break;
    case MODE_READ_FILTERED:
      canReadFiltered();
      break;
    default:
      // No default case
      break;
  }
}

void canReadAll() {
  if (mcp2515.readMessage(&canMsg) == MCP2515::ERROR_OK) {
    Serial.print(canMsg.can_id, HEX);
    Serial.print(" ");
    Serial.print(canMsg.can_dlc, HEX);
    Serial.print(" ");
    
    for (int i = 0; i < canMsg.can_dlc; i++) {  // print the data
      Serial.print(canMsg.data[i], HEX);
      Serial.print(" ");
    }

    Serial.println();
  }
}

void setupCanFilters() {
  char inputBuffer[10]; // Buffer for the input

  // Clear the Serial buffer
  while (Serial.available() > 0) {
    Serial.read();
  }

  // Read mask in hex
  Serial.println("Enter Mask in HEX (e.g., 7FF for all bits):");
  while (!Serial.available()) ; // Wait for user input
  Serial.readBytesUntil('\n', inputBuffer, sizeof(inputBuffer) - 1);
  inputBuffer[sizeof(inputBuffer) - 1] = 0; // Ensure null-termination
  uint32_t mask = strtol(inputBuffer, NULL, 16);

  while (Serial.available() > 0) {
    Serial.read();
  }

  // Read filter in hex
  Serial.println("Enter Filter in HEX (e.g., 201 for specific ID):");
  while (!Serial.available()) ;
  Serial.readBytesUntil('\n', inputBuffer, sizeof(inputBuffer) - 1);
  inputBuffer[sizeof(inputBuffer) - 1] = 0;
  uint32_t filter = strtol(inputBuffer, NULL, 16);

  Serial.print("Setting Mask: 0x");
  Serial.println(mask, HEX);
  Serial.print("Setting Filter: 0x");
  Serial.println(filter, HEX);

  mcp2515.setFilterMask(MCP2515::MASK0, false, mask);
  mcp2515.setFilter(MCP2515::RXF0, false, filter);
  mcp2515.setFilter(MCP2515::RXF1, false, filter); // Two filter buffer available with our setup
  // Add additional filters if necessary
}


void canReadFiltered() {
  if (mcp2515.readMessage(&canMsg) == MCP2515::ERROR_OK) {
    Serial.print(canMsg.can_id, HEX); // print ID
    Serial.print(" ");
    Serial.print(canMsg.can_dlc, HEX); // print DLC
    Serial.print(" ");
    
    for (int i = 0; i < canMsg.can_dlc; i++) {  // print the data
      Serial.print(canMsg.data[i], HEX);
      Serial.print(" ");
    }

    Serial.println();
  }
}

void prepareCanWrite() {
  char inputBuffer[10]; // Make sure this is large enough to hold the expected input plus a null terminator

  // Read CAN ID in hex (expects input in the format "0x201" or "201")
  Serial.readBytesUntil('\n', inputBuffer, sizeof(inputBuffer) - 1);
  canMsg.can_id = strtol(inputBuffer, NULL, 16);

  // Read CAN DLC in decimal
  Serial.readBytesUntil('\n', inputBuffer, sizeof(inputBuffer) - 1);
  canMsg.can_dlc = strtol(inputBuffer, NULL, 10);

  // Read CAN DATA in hex
  for (int i = 0; i < canMsg.can_dlc; i++) {
    Serial.readBytesUntil('\n', inputBuffer, sizeof(inputBuffer) - 1);
    canMsg.data[i] = strtol(inputBuffer, NULL, 16);
  }
  
  Serial.println("Custom message prepared.");
}

void canWrite() {
  mcp2515.sendMessage(&canMsg);
  // If you have a second message to send, call sendMessage() again with canMsg2
  
  Serial.println("Message sent");
  delay(1000); // Send every second
}

void canSpeedtest() {
  static int cntr = 0;
  static unsigned long oldTime = 0;

  if (mcp2515.readMessage(&canMsg) == MCP2515::ERROR_OK) {
    cntr++;
  }

  if ((millis() - oldTime) > 1000) {
    oldTime = millis();
    Serial.print(cntr);
    Serial.println(" msg/sec");
    cntr = 0;      
  }
}
