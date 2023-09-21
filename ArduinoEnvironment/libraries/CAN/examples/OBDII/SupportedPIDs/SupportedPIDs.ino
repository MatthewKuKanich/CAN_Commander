// Copyright (c) Sandeep Mistry. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.
//
//
// This examples queries the ECU for the Mode 01 OBD-II PID's it supports and prints the supported
// OBD-II PID's to the serial monitor
//
// A full list of PID's and their meaning can be found here:
//   https://en.wikipedia.org/wiki/OBD-II_PIDs#Mode_01
//
#include <CAN.h>

// Most cars support 11-bit adddress, others (like Honda),
// require 29-bit (extended) addressing, set the next line
// to true to use extended addressing
const bool useStandardAddressing = true;

void setup() {
  Serial.begin(9600);

  Serial.println("CAN OBD-II supported pids");

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
  for (int pid = 0x00; pid < 0xe0; pid += 0x20) {
    if (useStandardAddressing) {
      CAN.beginPacket(0x7df, 8);
    } else {
      CAN.beginExtendedPacket(0x18db33f1, 8);
    }
    CAN.write(0x02); // number of additional bytes
    CAN.write(0x01); // show current data
    CAN.write(pid);  // PID
    CAN.endPacket();

    // wait for response
    while (CAN.parsePacket() == 0 ||
           CAN.read() < 6 ||         // correct length
           CAN.read() != 0x41 ||     // correct mode
           CAN.read() != pid);       // correct PID

    unsigned long pidsSupported = 0;

    for (int i = 0; i < 4; i++) {
      pidsSupported <<= 8;
      pidsSupported |= CAN.read();
    }

    for (unsigned int i = 31; i > 0; i--) {
      if (pidsSupported & (1UL << i)) {
        int pidSupported = pid + (32 - i);

        Serial.print("0x");
        if (pidSupported < 16) {
          Serial.print("0");
        }
        Serial.println(pidSupported, HEX);
      }
    }

    if ((pidsSupported & 0x00000001) == 0x00000000) {
      // next round not supported, all done
      break;
    }
  }

  Serial.println("That's all folks!");

  while (1); // all done
}
