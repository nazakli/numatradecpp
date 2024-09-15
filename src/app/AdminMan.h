//
// Created by Numan on 8.09.2024.
//

#ifndef ADMIN_H
#define ADMIN_H
#include "../core/entity/Company.h"

class AdminMan {
    Company& company;
public:
    explicit AdminMan(Company& company) : company(company) {}
    [[nodiscard]] Company& getCompany() const {
        return company;
    }
};



#endif //ADMIN_H
