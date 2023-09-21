#include "CanHackerLineReader.h"

CanHackerLineReader::CanHackerLineReader(CanHacker *vCanHacker) {
    _canHacker = vCanHacker;
    index = 0;
    memset(buffer, 0, sizeof(buffer));
}

CanHacker::ERROR CanHackerLineReader::processChar(char rxChar) {
    switch (rxChar) {
        case '\r':
        case '\n':
            if (index > 0) {
                buffer[index] = '\0';
            
                CanHacker::ERROR error = _canHacker->receiveCommand(buffer, index);
                index = 0;
                if (error != CanHacker::ERROR_OK) {
                    return error;
                }
            }
            break;
  
        case '\0':
            break;
  
        default:
            if (index < COMMAND_MAX_LENGTH) {
                buffer[index++] = rxChar;
            } else {
                index = 0;
                return CanHacker::ERROR_INVALID_COMMAND;
            }
            break;
    }
    return CanHacker::ERROR_OK;
}

CanHacker::ERROR CanHackerLineReader::process() {
    Stream *stream = _canHacker->getInterfaceStream();
    while (stream->available()) {
        CanHacker::ERROR error = processChar(stream->read());
        if (error != CanHacker::ERROR_OK) {
            return error;
        }
    }
    
    return CanHacker::ERROR_OK;
}
