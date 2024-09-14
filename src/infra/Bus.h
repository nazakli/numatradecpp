//
// Created by Numan on 8.09.2024.
//

#ifndef BUS_H
#define BUS_H

#include <iostream>
#include <string>
#include <nats/nats.h>
#include "Logger.h"

class Bus {
    natsConnection* conn;

public:
    // Public connection pointer
    natsConnection* getConnection() {
        if (!conn) {
            natsStatus status = natsConnection_ConnectTo(&conn, NATS_DEFAULT_URL);
            if (status != NATS_OK) {
                Logger::error("Failed to connect to NATS: " + std::string(natsStatus_GetText(status)));
            } else {
                Logger::info("Connected to NATS server.");
            }
        }
        return conn;
    }

    // Singleton pattern to get the instance of Bus
    static Bus& getInstance() {
        static Bus instance;
        return instance;
    }

    // Constructor to initialize the NATS connection
    Bus() : conn(nullptr) {
        natsStatus s = natsConnection_ConnectTo(&conn, NATS_DEFAULT_URL);
        if (s == NATS_OK) {
            std::cout << "nats connected to server." << std::endl;
        } else {
            std::cerr << "nats failed to connect: " << natsStatus_GetText(s) << std::endl;
        }
    }

    // Destructor to clean up the NATS connection
    ~Bus() {
        if (conn != nullptr) {
            natsConnection_Destroy(conn);
            std::cout << "nats connection closed." << std::endl;
        }
    }

    // Publish method to send a message to a specific subject
    void publish(const std::string& subject, const std::string& message) {
        if (conn != nullptr) {
            natsStatus s = natsConnection_PublishString(conn, subject.c_str(), message.c_str());
            if (s == NATS_OK) {
                std::cout << "nats " << subject << " : " << message << std::endl;
            } else {
                std::cerr << "nats failed to publish message: " << natsStatus_GetText(s) << std::endl;
            }
        } else {
            std::cerr << "nats connection is not established." << std::endl;
        }
    }

    // Delete copy constructor and assignment operator to enforce singleton pattern
    Bus(const Bus&) = delete;
    Bus& operator=(const Bus&) = delete;
};

#endif // BUS_H