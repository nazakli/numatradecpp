#ifndef BUS_REQUESTER_H
#define BUS_REQUESTER_H

#include "BusInstance.h"
#include <string>
#include <iostream>
#include <thread>
#include <atomic>

class BusRequester {
private:
    std::string subject;

public:
    // Constructor, istek yapılacak kanalı alır
    BusRequester(const std::string& subject) : subject(subject) {}

    // İstek gönderme fonksiyonu (non-blocking)
    void request(const std::string& message, void (*callback)(natsConnection*, natsMsg*, void*)) {
        BusInstance* busInstance = BusInstance::getInstance();
        natsConnection* conn = busInstance->getConnection();

        // Asenkron istek gönderiliyor
        natsConnection_PublishRequestString(conn, subject.c_str(), message.c_str(), callback, nullptr);
        std::cout << "Request sent to subject: " << subject << " - Message: " << message << std::endl;
    }

    ~BusRequester() {
        std::cout << "BusRequester for subject: " << subject << " destroyed" << std::endl;
    }
};

#endif // BUS_REQUESTER_H