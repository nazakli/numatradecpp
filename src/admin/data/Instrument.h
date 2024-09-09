//
// Created by Numan on 8.09.2024.
//

#ifndef INSTRUMENT_H
#define INSTRUMENT_H
#include <string>

#include "enum/Mode.h"

struct Instrument {
    std::string code;
    std::string name;
    Mode mode;
    int digits;
};
#endif //INSTRUMENT_H
