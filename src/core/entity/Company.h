//
// Created by Numan on 8.09.2024.
//

#ifndef COMPANY_H
#define COMPANY_H
#include  <string>

#include "Entity.h"
#include "../../infra/Data/Db.h"

struct Company : Entity{
  std::string code;
  Company(const uint64_t id, std::string code):
    Entity(id, "company"), code(std::move(code)) {
    db.executeSQL("REPLACE INTO "+ tableName + " (id, code) VALUES (" + std::to_string(id) + ", '" + code + "')");
  }
};
#endif //COMPANY_H
