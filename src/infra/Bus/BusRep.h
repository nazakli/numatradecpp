#ifndef BUS_REPLIER_H
#define BUS_REPLIER_H

#include <iostream>
#include <string>
#include <utility>

#include "BusInstance.h"

class BusRep {
    natsSubscription* sub;
    std::string subject;

public:

    explicit BusRep(std::string  subject) : subject(std::move(subject)), sub(nullptr) {}

    void listen(void (*callback)(natsConnection*, natsSubscription*, natsMsg*, void*)) {
        const BusInstance* busInstance = BusInstance::getInstance();
        natsConnection* conn = busInstance->getConnection();

        natsConnection_Subscribe(&sub, conn, subject.c_str(), callback, nullptr);
        std::cout << "Listening for requests on subject: " << subject << std::endl;
    }
    ~BusRep() {
        if (sub != nullptr) {
            natsSubscription_Unsubscribe(sub);
            natsSubscription_Destroy(sub);
            std::cout << "Stopped listening on subject: " << subject << std::endl;
        }
    }
};

#endif // BUS_REPLIER_H