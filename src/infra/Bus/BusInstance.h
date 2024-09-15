//
// Created by Numan on 15.09.2024.
//

#ifndef BUS_H
#define BUS_H
#ifndef BUS_INSTANCE_H
#define BUS_INSTANCE_H

#include <nats/nats.h>
#include <mutex>

class BusInstance {
private:
    static BusInstance* instance;
    static std::mutex instanceMutex;
    natsConnection* conn;
    natsOptions* opts;

    // Private constructor to prevent direct instantiation
    BusInstance(const std::string& url) {
        natsOptions_Create(&opts);
        natsOptions_SetURL(opts, url.c_str());

        // Bağlantı kuruluyor
        natsConnection_Connect(&conn, opts);
    }

public:
    // Deleted methods to ensure the class is singleton
    BusInstance(const BusInstance&) = delete;
    void operator=(const BusInstance&) = delete;

    // Singleton instance'ı döndüren fonksiyon
    static BusInstance* getInstance(const std::string& url = "nats://localhost:4222") {
        std::lock_guard<std::mutex> lock(instanceMutex);
        if (!instance) {
            instance = new BusInstance(url);
        }
        return instance;
    }

    // NATS bağlantısını döndüren fonksiyon
    natsConnection* getConnection() {
        return conn;
    }

    ~BusInstance() {
        natsConnection_Close(conn);
        natsConnection_Destroy(conn);
        natsOptions_Destroy(opts);
    }
};

// Initialize static members
BusInstance* BusInstance::instance = nullptr;
std::mutex BusInstance::instanceMutex;

#endif //BUS_H
