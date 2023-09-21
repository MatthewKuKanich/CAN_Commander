#include <can.h>
#include <mcp2515.h>

#include <CanHacker.h>
#include <CanHackerLineReader.h>
#include <lib.h>

#include <SPI.h>
#include <SoftwareSerial.h>

const int SPI_CS_PIN = 10;
const int INT_PIN = 2;

const int SS_RX_PIN = 3;
const int SS_TX_PIN = 4;

CanHackerLineReader *lineReader = NULL;
CanHacker *canHacker = NULL;

SoftwareSerial softwareSerial(SS_RX_PIN, SS_TX_PIN);

void handleError(const CanHacker::ERROR error);

void setup() {
    Serial.begin(115200);
    SPI.begin();
    softwareSerial.begin(115200);

    Stream *interfaceStream = &Serial;
    Stream *debugStream = &softwareSerial;
    
    
    canHacker = new CanHacker(interfaceStream, debugStream, SPI_CS_PIN);
    canHacker->enableLoopback(); // remove to disable loopback test mode
    lineReader = new CanHackerLineReader(canHacker);
    
    pinMode(INT_PIN, INPUT);
}

void loop() {
    CanHacker::ERROR error;
    
    if (digitalRead(INT_PIN) == LOW) {
        error = canHacker->processInterrupt();
        handleError(error);
    }
    
    // uncomment that lines for Leonardo, Pro Micro or Esplora
    // error = lineReader->process();
    // handleError(error);
}

// serialEvent handler not supported by Leonardo, Pro Micro and Esplora
void serialEvent() {
    CanHacker::ERROR error = lineReader->process();
    handleError(error);
}

void handleError(const CanHacker::ERROR error) {

    switch (error) {
        case CanHacker::ERROR_OK:
        case CanHacker::ERROR_UNKNOWN_COMMAND:
        case CanHacker::ERROR_NOT_CONNECTED:
        case CanHacker::ERROR_MCP2515_ERRIF:
        case CanHacker::ERROR_INVALID_COMMAND:
            return;

        default:
            break;
    }
  
    softwareSerial.print("Failure (code ");
    softwareSerial.print((int)error);
    softwareSerial.println(")");
  
    while (1) {
        delay(2000);
    } ;
}
