//
// Created by Numan on 8.09.2024.
//

#ifndef ADMIN_H
#define ADMIN_H
#include "../core/entity/Company.h"
#include "../infra/Bus.h"
#include "../infra/NatsBase.h"

class AdminMan : public NatsBase {
    Company& company;
public:
    explicit AdminMan(Company& company) : company(company) {
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
