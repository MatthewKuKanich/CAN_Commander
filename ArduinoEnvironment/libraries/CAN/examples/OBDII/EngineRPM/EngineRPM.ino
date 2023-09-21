// Copyright (c) Sandeep Mistry. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.
//
//
// This examples queries the engine RPM (OBD-II PID 0x0c) once a seconds and
// prints the value to the serial monitor
//
#include <CAN.h>

// Most cars support 11-bit adddress, others (like Honda),
// require 29-bit (extended) addressing, set the next line
// to true to use extended addressing
const bool useStandardAddressing = true;

void setup() {
  Serial.begin(9600);

  Serial.println("CAN OBD-II engine RPM");

  // start the CAN bus at 500 kbps
  if (!CAN.begin(500E3)) {
    Serial.println("Starting CAN failed!");
    while (1);
  }

  // add filter to only receive the CAN bus ID's we care about
  if (useStandardAddressing) {
    CAN.filter(0x7e8);
  } else {
    CAN.filterExtended(0x18daf110);
  }
}

void loop() {
  if (useStandardAddressing) {
    CAN.beginPacket(0x7df, 8);
  } else {
    CAN.beginExtendedPacket(0x18db33f1, 8);
  }
  CAN.write(0x02); // number of additional bytes
  CAN.write(0x01); // show current data
  CAN.write(0x0c); // engine RPM
  CAN.endPacket();

  // wait for response
  while (CAN.parsePacket() == 0 ||
         CAN.read() < 3 ||          // correct length
         CAN.read() != 0x41 ||      // correct mode
         CAN.read() != 0x0c);       // correct PID

  float rpm = ((CAN.read() * 256.0) + CAN.read()) / 4.0;

  Serial.print("Engine RPM = ");
  Serial.println(rpm);

  delay(1000);
}
