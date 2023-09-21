#include <can.h>
#include <mcp2515.h>

#include <CanHacker.h>
#include <CanHackerLineReader.h>
#include <lib.h>

#include <SPI.h>

const int SPI_CS_PIN = 10;
const int INT_PIN = 2;

CanHackerLineReader *lineReader = NULL;
CanHacker *canHacker = NULL;

void setup() {
    Serial.begin(115200);
    SPI.begin();
    
    canHacker = new CanHacker(&Serial, NULL, SPI_CS_PIN);
    lineReader = new CanHackerLineReader(canHacker);
    
    pinMode(INT_PIN, INPUT);
}

void loop() {
    if (digitalRead(INT_PIN) == LOW) {
        canHacker->processInterrupt();
    }

    // uncomment that lines for Leonardo, Pro Micro or Esplora
    // if (Serial.available()) {
    //   lineReader->process();    
    // }
}

// serialEvent handler not supported by Leonardo, Pro Micro and Esplora
void serialEvent() {
    lineReader->process();
}
