#ifndef NATS_LOGGER_HPP
#define NATS_LOGGER_HPP

#include <nats/nats.h>
#include "Logger.h"
#include "Bus.h"
#include <thread>

class NATSLogger {
public:
    // Constructor
    NATSLogger() {}

    // Initialize and start the subscriber
    void start() {
        subscriberThread = std::thread(&NATSLogger::subscribeToNATS, this);
    }

    // Wait for subscriber thread to finish
    void wait() {
        subscriberThread.join();
    }

    // Send a request to a specified subject
    void sendRequest(const std::string &subject, const std::string &message) {
        natsStatus status;
        natsMsg *replyMsg = nullptr;
        natsConnection* conn = Bus::getInstance().getConnection();

        // Send the request
        status = natsConnection_RequestString(&replyMsg, conn, subject.c_str(), message.c_str(), 1000);
        if (status == NATS_OK) {
            Logger::info("Received reply to request: " + std::string(natsMsg_GetData(replyMsg)));
        } else {
            Logger::error("Failed to send request or receive reply: " + std::string(natsStatus_GetText(status)));
        }

        // Cleanup
        natsMsg_Destroy(replyMsg);
    }

private:
    std::thread subscriberThread;

    void subscribeToNATS() {
        natsStatus status;
        natsSubscription *sub = nullptr;
        natsConnection* conn = Bus::getInstance().getConnection();

        // Subscribe to all subjects (* indicates all subjects)
        status = natsConnection_Subscribe(&sub, conn, "*", onMessage, nullptr);
        if (status != NATS_OK) {
            Logger::error("Failed to subscribe to NATS: " + std::string(natsStatus_GetText(status)));
            return;
        }

        Logger::info("Subscribed to all subjects.");

        // Keep the connection open to continue receiving messages
        while (true) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }

        // Cleanup (though not realistically reachable in this example)
        natsSubscription_Destroy(sub);
    }

    static void onMessage(natsConnection *nc, natsSubscription *sub, natsMsg *msg, void *closure) {
        // Log the received message
        Logger::info("nats " + std::string(natsMsg_GetSubject(msg)) + " : " + std::string(natsMsg_GetData(msg)));

        // Handle replies if necessary
        const char *reply = natsMsg_GetReply(msg);
        if (reply != nullptr) {
            // Example: Echo the message back
            natsStatus s = natsConnection_PublishString(nc, reply, "This is a reply");
            if (s == NATS_OK) {
                Logger::info("Sent reply: This is a reply");
            } else {
                Logger::error("Failed to publish reply: " + std::string(natsStatus_GetText(s)));
            }
        }
        natsMsg_Destroy(msg);
    }
};

#endif // NATS_LOGGER_HPP