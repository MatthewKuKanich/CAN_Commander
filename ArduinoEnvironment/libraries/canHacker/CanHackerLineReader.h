#ifndef CANHACKERLINEREADER_H_
#define CANHACKERLINEREADER_H_

#include "CanHacker.h"

class CanHackerLineReader {
    private:
        static const int COMMAND_MAX_LENGTH = 30; // not including \r\0
        
        CanHacker *_canHacker;
        char buffer[COMMAND_MAX_LENGTH + 2];
        int index;
        Stream *_stream;
    public:
        CanHackerLineReader(CanHacker *vCanHacker);
        CanHacker::ERROR processChar(char rxChar);
        CanHacker::ERROR process();
};

#endif /* CANHACKERLINEREADER_H_ */
