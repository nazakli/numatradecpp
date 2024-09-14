//
// Created by Numan on 14.09.2024.
//
#include <vector>
#include "Account.h"
#include "Instrument.h"
#ifndef GROUP_H
#define GROUP_H
struct Group {
    std::vector<Account> accounts;
    std::string name;
    Instrument currency;
    double marginCall;
    double stopOut;
};
#endif //GROUP_H
