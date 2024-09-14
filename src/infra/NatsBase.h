#ifndef NATSBASE_H
#define NATSBASE_H

#include <iostream>
#include <string>
#include <nats/nats.h>
#include <functional>
#include "Bus.h"
#include "Ipc.h"

class NatsBase : public IpcBase {
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
    void pubBus(const std::string& subject, const std::string& message) const {
        try {
            bus.publish(subject, message);
            std::cout << "NatsBase Published message to subject: " << subject << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "NatsBase Failed to publish message: " << e.what() << std::endl;
            throw;
        }
    }

    // Subscribe to a subject with flexible callback handling using std::function
    void subBus(const std::string& subject, std::function<void(natsMsg*)> onMessage, void* closure = nullptr) {
        natsSubscription* sub = nullptr;
        auto wrapper = [](natsConnection* nc, natsSubscription* sub, natsMsg* msg, void* closure) {
            auto* handler = static_cast<std::function<void(natsMsg*)>*>(closure);
            (*handler)(msg);
        };

        natsStatus s = natsConnection_Subscribe(&(sub), bus.getConnection(), subject.c_str(), wrapper, &onMessage);
        if (s == NATS_OK) {
            std::cout << "NatsBase Subscribed to subject: " << subject << std::endl;
        } else {
            std::cerr << "NatsBase Failed to subscribe: " << natsStatus_GetText(s) << std::endl;
        }
    }

    // Send a request and get a reply, processing the reply via callback
    void reqBus(const std::string& subject, const std::string& message, std::function<void(const std::string&)> onReply, int timeout = 5000) {
        natsMsg* replyMsg = nullptr;
        try {
            natsStatus s = natsConnection_RequestString(&(replyMsg), bus.getConnection(), subject.c_str(), message.c_str(), timeout);
            if (s == NATS_OK) {
                std::string replyData = natsMsg_GetData(replyMsg);
                std::cout << "NatsBase Received reply: " << replyData << std::endl;
                // Callback fonksiyonu ile cevabı işle
                onReply(replyData);
            } else {
                std::cerr << "NatsBase Request failed: " << natsStatus_GetText(s) << std::endl;
            }
        } catch (const std::exception& e) {
            std::cerr << "NatsBase Exception in request: " << e.what() << std::endl;
            throw;
        }
        natsMsg_Destroy(replyMsg);
    }

    // Reply to a request message with flexible callback handling using std::function
    void repBus(const std::string& subject, std::function<void(natsMsg*)> onRequest, void* closure = nullptr) {
        natsSubscription* sub = nullptr;
        auto wrapper = [](natsConnection* nc, natsSubscription* sub, natsMsg* msg, void* closure) {
            auto* handler = static_cast<std::function<void(natsMsg*)>*>(closure);
            (*handler)(msg);
        };

        natsStatus s = natsConnection_Subscribe(&(sub), bus.getConnection(), subject.c_str(), wrapper, &onRequest);
        if (s == NATS_OK) {
            std::cout << "NatsBase Listening for requests on subject: " << subject << std::endl;
        } else {
            std::cerr << "NatsBase Failed to subscribe for requests: " << natsStatus_GetText(s) << std::endl;
        }
    }
};

#endif // NATSBASE_H