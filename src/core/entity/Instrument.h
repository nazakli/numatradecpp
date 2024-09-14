//
// Created by Numan on 8.09.2024.
//

#ifndef INSTRUMENT_H
#define INSTRUMENT_H
#include <string>

#include "../enum/InstrumentType.h"

struct Instrument {
    std::string code;
    std::string name;
    InstrumentType instrumentType;
    int digits;
};
#endif //INSTRUMENT_H
