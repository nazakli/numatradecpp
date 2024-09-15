#ifndef BUS_PUBLISHER_H
#define BUS_PUBLISHER_H

#include "BusInstance.h"
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <string>

class BusPublisher {
private:
    std::queue<std::string> messageQueue;
    std::mutex queueMutex;
    std::condition_variable cv;
    std::atomic<bool> stopFlag;
    std::thread publisherThread;

public:
    BusPublisher() : stopFlag(false) {
        publisherThread = std::thread(&BusPublisher::processQueue, this);
    }

    ~BusPublisher() {
        stopFlag = true;
        cv.notify_all();
        if (publisherThread.joinable()) {
            publisherThread.join();
        }
    }

    void publish(const std::string& subject, const std::string& message) {
        std::lock_guard<std::mutex> lock(queueMutex);
        messageQueue.push(subject + ":" + message);
        cv.notify_one();
    }

private:
    void processQueue() {
        BusInstance* busInstance = BusInstance::getInstance();

        while (!stopFlag) {
            std::unique_lock<std::mutex> lock(queueMutex);
            cv.wait(lock, [this]() { return !messageQueue.empty() || stopFlag; });
            if (!messageQueue.empty()) {
                std::string message = messageQueue.front();
                messageQueue.pop();
                lock.unlock();

                std::string subject = message.substr(0, message.find(":"));
                std::string content = message.substr(message.find(":") + 1);
                natsConnection_PublishString(busInstance->getConnection(), subject.c_str(), content.c_str());
            }
        }
    }
};

#endif // BUS_PUBLISHER_H