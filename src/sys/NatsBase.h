#ifndef NATSBASE_H
#define NATSBASE_H

#include <iostream>
#include <string>
#include <nats/nats.h>
#include "Bus.h"

class NatsBase {
protected:
    // Use the Bus singleton to access the NATS connection
    Bus& bus = Bus::getInstance();

public:
    NatsBase() {
        std::cout << "NatsBase initialized." << std::endl;
    }

    virtual ~NatsBase() {
        std::cout << "NatsBase destroyed." << std::endl;
    }

    // Publish message to a subject
    void publish(const std::string& subject, const std::string& message) const {
        bus.publish(subject, message);
    }

    // Subscribe to a subject
    void subscribe(const std::string& subject, natsMsgHandler onMessage, void* closure = nullptr) {
        natsSubscription* sub = nullptr;
        natsStatus s = natsConnection_Subscribe(&(sub), bus.getConnection(), subject.c_str(), onMessage, closure);
        if (s == NATS_OK) {
            std::cout << "Subscribed to subject: " << subject << std::endl;
        } else {
            std::cerr << "Failed to subscribe: " << natsStatus_GetText(s) << std::endl;
        }
    }

    // Send a request and get a reply
    void request(const std::string& subject, const std::string& message, int timeout = 5000) {
        natsMsg* replyMsg = nullptr;
        natsStatus s = natsConnection_RequestString(&(replyMsg), bus.getConnection(), subject.c_str(), message.c_str(), timeout);
        if (s == NATS_OK) {
            std::cout << "Received reply: " << natsMsg_GetData(replyMsg) << std::endl;
        } else {
            std::cerr << "Request failed: " << natsStatus_GetText(s) << std::endl;
        }
        natsMsg_Destroy(replyMsg);
    }

    // Reply to a request message
    void reply(const std::string& subject, natsMsgHandler onRequest, void* closure = nullptr) {
        natsSubscription* sub = nullptr;
        natsStatus s = natsConnection_Subscribe(&(sub), bus.getConnection(), subject.c_str(), onRequest, closure);
        if (s == NATS_OK) {
            std::cout << "Listening for requests on subject: " << subject << std::endl;
        } else {
            std::cerr << "Failed to subscribe for requests: " << natsStatus_GetText(s) << std::endl;
        }
    }
};

#endif // NATSBASE_H