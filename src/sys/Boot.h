//
// Created by Numan on 9.09.2024.
//

#ifndef BOOT_H
#define BOOT_H
#include "NatsBase.h"

class Boot : NatsBase{
public:
    explicit Boot() {
        publish("boot", "Boot is created.");
        publish("boot", "Boot is initialized.");
    }
    ~Boot() {
        publish("boot", "Boot is destroyed.");
    }

    int start() {
        publish("boot", "Boot is starting.");

        return 0;
    }
};
#endif //BOOT_H
