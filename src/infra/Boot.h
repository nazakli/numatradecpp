//
// Created by Numan on 9.09.2024.
//

#ifndef BOOT_H
#define BOOT_H
#include "NatsBase.h"
#include "../app/AdminMan.h"
#include "../core/enum/InstrumentType.h"

class Boot : NatsBase{
public:
    explicit Boot() {
        pubBus("boot", "Boot is created.");
        pubBus("boot", "Boot is initialized.");
    }
    ~Boot() {
        pubBus("boot", "Boot is destroyed.");
    }

    int start() {
        pubBus("boot", "Boot is starting.");
        reqBus("boot.company.details", "company.details", [](const std::string& reply) {
            std::cout << "Received reply from: " << reply << std::endl;
        });
        Instrument instrument;
        instrument.code = "USD";
        instrument.name = "US Dollar";
        instrument.instrumentType = InstrumentType::CASH;
        instrument.digits = 2;

        Company company;
        company.code = "C1";
        company.name = "Company Name C1";
        company.currency = instrument;

        AdminMan admin(company);
        pubBus("boot", "Boot is started.");

        return 0;
    }
};
#endif //BOOT_H
