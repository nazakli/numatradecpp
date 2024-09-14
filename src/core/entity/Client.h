//
// Created by Numan on 14.09.2024.
//

#ifndef CLIENT_H
#define CLIENT_H
#include <vector>

#include "Account.h"
#include "Instrument.h"

struct Client {
    std::vector<Account> accounts;
    Instrument currency;
};
#endif //CLIENT_H
