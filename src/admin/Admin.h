//
// Created by Numan on 8.09.2024.
//

#ifndef ADMIN_H
#define ADMIN_H
#include "data/Company.h"
#include "../sys/Bus.h"
#include "../sys/NatsBase.h"

class Admin : public NatsBase {
    Company& company;
public:
    explicit Admin(Company& company) : company(company) {
        publish("admin", "Admin is created.");
        publish("company", company.code + " is created.");
    }
    [[nodiscard]] Company& getCompany() const {
        return company;
    }
};



#endif //ADMIN_H
