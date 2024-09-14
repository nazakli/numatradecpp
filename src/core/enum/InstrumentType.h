//
// Created by Numan on 8.09.2024.
//

#ifndef MODE_H
#define MODE_H
enum InstrumentType {
    CASH=0, //Volume in lots * Contract size
    FOREX=1, //Volume in lots * Contract size / Leverage
    CFD=2, //Volume in lots * Contract size * Open market price
    CFD_LEVERAGE=3, // Volume in lots * Contract size * Open market price / Leverage
    CFD_INDEX=4, //Volume in lots * Contract size * Open market price * Tick value / Tick size
    FUTURES=5, //Volume in lots * Initial margin  buna ek olarak Volume in lots * Maintenance margin
    FIXED=6, // Volume in lots * Initial margin
};
#endif //MODE_H
