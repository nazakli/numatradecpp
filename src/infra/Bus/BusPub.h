#ifndef BUS_PUB_H
#define BUS_PUB_H

#include "BusInstance.h"
#include <queue>
#include <thread>
#include <condition_variable>
#include <atomic>
#include <string>

class BusPub {
    std::queue<std::string> messageQueue;
    std::mutex queueMutex;
    std::condition_variable cv;
    std::atomic<bool> stopFlag;
    std::thread publisherThread;

public:
    BusPub() : stopFlag(false) {
        publisherThread = std::thread(&BusPub::processQueue, this);
    }

    ~BusPub() {
        stopFlag = true;
        cv.notify_all();
        if (publisherThread.joinable()) {
            publisherThread.join();
        }
    }

    void publish(const std::string& subject, const std::string& message) {
        std::lock_guard<std::mutex> lock(queueMutex);
        messageQueue.push(subject + "|" + message);
        cv.notify_one();
    }

private:
    void processQueue() {
        const BusInstance* busInstance = BusInstance::getInstance();

        while (!stopFlag) {
            std::unique_lock<std::mutex> lock(queueMutex);
            cv.wait(lock, [this]() { return !messageQueue.empty() || stopFlag; });
            if (!messageQueue.empty()) {
                std::string message = messageQueue.front();
                messageQueue.pop();
                lock.unlock();

                std::string subject = message.substr(0, message.find("|"));
                std::string content = message.substr(message.find("|") + 1);
                natsConnection_PublishString(busInstance->getConnection(), subject.c_str(), content.c_str());
            }
        }
    }
};

#endif // BUS_PUB_H