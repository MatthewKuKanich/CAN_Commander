# CAN Commander: CAN Bus Reverse Engineering Tool

## Introduction
CAN Commander is a comprehensive tool designed for the reverse engineering of CAN (Controller Area Network) bus systems. This project aims to provide a robust platform for automotive enthusiasts, engineers, and security researchers to interact with and analyze CAN networks, facilitating a deeper understanding and manipulation of vehicular communication systems.

## Features
- **Message Injection**: Send custom CAN messages to test responses from different modules.
- **Message Logging**: Record and log CAN traffic for analysis.
- **Network Sniffing**: Monitor the CAN network to observe communication between different components.
- **Message Decoding**: Decode CAN messages and understand the underlying data structures.
- **Man-in-the-Middle Capabilities**: Use as a set and forget MITM device to do in-place packet swapping.
- **User-friendly Interface**: An intuitive interface for interacting with the CAN network. -App Version
- **Real-time Data Visualization**: Graphical representation of the CAN traffic for easier analysis.
- **DTC and Diagnostics**: Get all the features of a standard OBDII PID scanner
- **Wireless Options**: Communicate via wire tap, WiFi, or Bluetooth Low-Energy (BLE)

## Getting Started
To get started with CAN Commander, follow these steps:

1. **Clone the Repository**:

2. **Setup and Installation**:
- Install Android application (iOS App coming very soon)
- Clone and install autowp's mcp2515 library: https://github.com/autowp/arduino-mcp2515
- Grab a compatible microcontroller and mcp2515 module (recommendations below)
- Flash the microcontroller and hookup the mcp2515 to it via SPI
- Connect your CAN High and Low wires or plug in an OBDII adapter

3. **Running the Tool**:
- Be sure to flash the CAN_Commander arduino script.
- If using the android app flash CAN_WiFi to an ESP32
- Follow terminal based prompts

## Usage
- **Basic Operations**: Usage guide will be updated soon with indepth tutorials, currently a WIP
- How to send a message.
- How to log and analyze traffic.
- **Advanced Features**:
- Detailed guides on using advanced features like message decoding soon!

## Hardware Requirements
- Microcontroller (Arduino Uno, Mega, Nano, Pico, ESP8266, ESP32)
- MCP2515 (Make sure you verify the crystal 8/16/etc)

## Software Dependencies
- Arduino IDE
- USB, WiFi, or Bluetooth Serial Terminal

## Contributing
I welcome contributions to the CAN Commander project! I am currently working on the contributing guide and related documents. I'll moderate PRs regularly until this is finished so you can still contribute.

## Acknowledgments
- Thanks to autowp for the amazing mcp2515 library: https://github.com/autowp/arduino-mcp2515
