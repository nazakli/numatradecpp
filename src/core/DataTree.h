//
// Created by Numan on 18.09.2024.
//

#ifndef DATATREE_H
#define DATATREE_H
#include <unordered_map>

#include "entity/Company.h"
#include "entity/Client.h"
#include "entity/Exchange.h"
#include "entity/Group.h"

std::unordered_map<std::string, Company> companyMap; // apx.
std::unordered_map<int, Client> clientMap; // apx.10001
std::unordered_map<int, Group> groupMap; // apx.10001.1.
std::unordered_map<int, Account> accountMap; // apx.10001.1.8003443
std::unordered_map<int, Position> positionMap; // apx.10001.1.8003443.AAPL

std::unordered_map<std::string, Exchange> exchangeMap; // nasdaq
std::unordered_map<std::string, Instrument> instrumentMap; // nasdaq.AAPL








#endif //DATATREE_H
