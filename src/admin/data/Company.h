//
// Created by Numan on 8.09.2024.
//

#ifndef COMPANY_H
#define COMPANY_H
#include  <string>

#include "Instrument.h"

struct Company{
  std::string code;
  std::string name;
  Instrument currency;
};
#endif //COMPANY_H
