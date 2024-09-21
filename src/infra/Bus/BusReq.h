#ifndef BUS_REQUESTER_H
#define BUS_REQUESTER_H

#include <iostream>
#include <string>
#include <utility>

#include "BusInstance.h"

class BusReq {
private:
    std::string subject;

public:
    explicit BusReq(std::string  subject) : subject(std::move(subject)) {}


    void request(const std::string& message, void (*callback)(natsConnection*, natsMsg*, void*)) {
        BusInstance* busInstance = BusInstance::getInstance();
        natsConnection* conn = busInstance->getConnection();

        natsConnection_PublishRequestString(conn, subject.c_str(), message.c_str(), callback, nullptr);
        std::cout << "Request sent to subject: " << subject << " - Message: " << message << std::endl;
    }

    ~BusReq() {
        std::cout << "BusRequester for subject: " << subject << " destroyed" << std::endl;
    }
};

#endif // BUS_REQUESTER_H