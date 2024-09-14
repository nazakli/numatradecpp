//
// Created by Numan on 9.09.2024.
//

#ifndef BOOT_H
#define BOOT_H
#include "NatsBase.h"

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
            std::cout << "Received reply: " << reply << std::endl;
        });
        return 0;
    }
};
#endif //BOOT_H
