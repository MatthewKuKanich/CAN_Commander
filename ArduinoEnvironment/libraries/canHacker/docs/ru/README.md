# CanHacker (lawicel) CAN адаптер на Arduino + MCP2515

Используется эту [Библиотеку](https://github.com/autowp/arduino-mcp2515) для связи с MCP2515

## Возможности

Резлизует взаимодействие с CAN шиной через микросхему MCP2515 по протоколу CanHacker (lawicel).

- передача и прием can фреймов
- поддержка стандартных (11 бит) и расширенных (29 бит) фреймов
- поддержка remote фреймов (RTR)
- поддержка фильтра по ID (маска + код)
- интерфейс, работающий со [Stream](https://www.arduino.cc/en/Reference/Stream): возможность работы с Serial, SoftwareSerial, Ethernet и другими интерфейсами 
- поддерживаемые скорости can шины от 10Kbps до 1Mbps
- поддерживаются модули с разными кварцами - 8, 16, 20 МГц. По умолчанию установлена частота 16 МГц, используйте функцию setClock если у вас модуль с другим кварцем.
- поддержка [CanHacker](http://www.mictronics.de/projects/usb-can-bus/) (приложение для Windows)
- поддержка [CANreader](https://github.com/autowp/CANreader) (приложение для Android)

## Установка библиотеки

1. Установите [библиотеку MCP2515](https://github.com/autowp/arduino-mcp2515)
2. Скачайте ZIP архив https://github.com/autowp/arduino-canhacker/archive/master.zip
3. В меню Arduino IDE: Sketch -> Include Library... -> Add .ZIP Library...
4. Перезапустите Arduino IDE, чтобы увидеть "canhacker" в списке библиотек и примеров

Протестировано с Arduino Nano.
На Arduino Uno при работе через CanHacker для Windows, есть проблема с слишком долгой перезагрузкой и потерей первой комманды

## Использование

Пример

```
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
    while (!Serial);
    SPI.begin();
    softwareSerial.begin(115200);

    Stream *interfaceStream = &Serial;
    
    canHacker = new CanHacker(interfaceStream, null, SPI_CS_PIN);
    lineReader = new CanHackerLineReader(canHacker);
    
    pinMode(INT_PIN, INPUT);
}

void loop() {
    canHacker->processInterrupt();
    lineReader->process();
}
```

## Протокол

Протокол CanHacker (lawicel) описан в [документации к приложению CanHacker](http://www.mictronics.de/projects/usb-can-bus/)

Библиотека реализует его не полностью. [Поддерживаемые комманды приведены здесь](protocol.md).

## Содействие

Приветствуются любые способы участия в коде.