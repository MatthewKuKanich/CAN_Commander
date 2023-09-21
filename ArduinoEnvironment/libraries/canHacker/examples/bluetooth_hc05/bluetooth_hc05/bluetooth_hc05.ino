#include <can.h>
#include <mcp2515.h>

#include <CanHacker.h>
#include <CanHackerLineReader.h>
#include <lib.h>

#include <SPI.h>

const int SPI_CS_PIN = 53; // 10 by default, 53 on mega
const int INT_PIN = 2;

CanHackerLineReader *lineReader = NULL;
CanHacker *canHacker = NULL;

void setup() {
    Serial.begin(115200);
    while (!Serial);
    Serial1.begin(57600);
    while (!Serial1);
    SPI.begin();

    Stream *interfaceStream = &Serial1;
    Stream *debugStream = &Serial;
    
    
    canHacker = new CanHacker(interfaceStream, debugStream, SPI_CS_PIN);
    //canHacker->enableLoopback(); // uncomment this for loopback
    lineReader = new CanHackerLineReader(canHacker);
    
    pinMode(INT_PIN, INPUT);
}

void loop() {
    CanHacker::ERROR error;
  
    if (digitalRead(INT_PIN) == LOW) {
        error = canHacker->processInterrupt();
        handleError(error);
    }

    error = lineReader->process();
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
  
    Serial.print("Failure (code ");
    Serial.print((int)error);
    Serial.println(")");

    digitalWrite(SPI_CS_PIN, HIGH);
    pinMode(LED_BUILTIN, OUTPUT);
  
    while (1) {
        int c = (int)error;
        for (int i=0; i<c; i++) {
            digitalWrite(LED_BUILTIN, HIGH);
            delay(500);
            digitalWrite(LED_BUILTIN, LOW);
            delay(500);
        }
        
        delay(2000);
    } ;
}
