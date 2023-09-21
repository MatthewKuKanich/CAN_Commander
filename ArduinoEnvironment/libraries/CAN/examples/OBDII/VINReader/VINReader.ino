// Copyright (c) Sandeep Mistry. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.
//
//
// This examples queries the ECU for the car's Vehicle Identification Number (VIN) and
// prints it out to the serial monitor using Mode 09 and OBD-II PID 0x02
//
#include <CAN.h>

// Most cars support 11-bit adddress, others (like Honda),
// require 29-bit (extended) addressing, set the next line
// to true to use extended addressing
const bool useStandardAddressing = true;

void setup() {
  Serial.begin(9600);

  Serial.println("CAN OBD-II VIN reader");

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
  // send the request for the first chunk
  if (useStandardAddressing) {
    CAN.beginPacket(0x7df, 8);
  } else {
    CAN.beginExtendedPacket(0x18db33f1, 8);
  }
  CAN.write(0x02); // Number of additional bytes
  CAN.write(0x09); // Request vehicle information
  CAN.write(0x02); // Vehicle Identification Number (VIN)
  CAN.endPacket();

  // wait for response
  while (CAN.parsePacket() == 0 ||
         CAN.read() != 0x10 || CAN.read() != 0x14 || // correct length
         CAN.read() != 0x49 ||                       // correct mode
         CAN.read() != 0x02 ||                       // correct PID
         CAN.read() != 0x01);

  // print out
  while (CAN.available()) {
    Serial.write((char)CAN.read());
  }

  // read in remaining chunks
  for (int i = 0; i < 2; i++) {
    // send the request for the next chunk
    if (useStandardAddressing) {
      CAN.beginPacket(0x7e0, 8);
    } else {
      CAN.beginExtendedPacket(0x18db33f1, 8);
    }
    CAN.write(0x30);
    CAN.endPacket();

    // wait for response
    while (CAN.parsePacket() == 0 ||
           CAN.read() != (0x21 + i)); // correct sequence number

    // print out
    while (CAN.available()) {
      Serial.write((char)CAN.read());
    }
  }

  Serial.println("That's all folks!");

  while (1); // all done
}
