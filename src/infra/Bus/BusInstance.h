//
// Created by Numan on 15.09.2024.
//

#ifndef BUS_INSTANCE_H
#define BUS_INSTANCE_H

#include <nats/nats.h>
#include <mutex>
class BusInstance {
    static std::unique_ptr<BusInstance> instance;
    static std::mutex instanceMutex;
    natsConnection* conn;
    natsOptions* opts;

    explicit BusInstance(const std::string& url) {
        natsOptions_Create(&opts);
        natsOptions_SetURL(opts, url.c_str());
        natsConnection_Connect(&conn, opts);
    }

public:
    BusInstance(const BusInstance&) = delete;
    void operator=(const BusInstance&) = delete;

    static BusInstance* getInstance(const std::string& url = "nats://localhost:4222") {
        std::lock_guard<std::mutex> lock(instanceMutex);
        if (!instance) {
            instance = std::unique_ptr<BusInstance>(new BusInstance(url));
        }
        return instance.get();
    }

    [[nodiscard]] natsConnection* getConnection() const {
        return conn;
    }

    ~BusInstance() {
        natsConnection_Close(conn);
        natsConnection_Destroy(conn);
        natsOptions_Destroy(opts);
    }
};

// Initialize static members
std::unique_ptr<BusInstance> BusInstance::instance = nullptr;
std::mutex BusInstance::instanceMutex;

#endif //BUS_INSTANCE_H
