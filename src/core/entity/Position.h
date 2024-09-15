//
// Created by Numan on 14.09.2024.
//

#ifndef POSITION_H
#define POSITION_H
#include "Instrument.h"
#include "../enum/PositionSide.h"

struct Position {
    Instrument instrument;
    int64_t id;
    double qty=0;
    double openPrice=0;
    PositionSide side;
    double last_price=0;
    double profit=0;
};
#endif //POSITION_H
