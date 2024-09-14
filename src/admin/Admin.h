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
        pubBus("admin", "Admin is created.");
        IpcMsg msg("INFO", "{Admin is created.}");
        bool stat = pubIpc(msg);
        pubBus("company", company.code + " is created.");
    }
    [[nodiscard]] Company& getCompany() const {
        return company;
    }
};



#endif //ADMIN_H
