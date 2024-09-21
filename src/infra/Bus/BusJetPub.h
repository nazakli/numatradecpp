#ifndef BUS_JET_PUB_H
#define BUS_JET_PUB_H

#include <queue>
#include <thread>
#include <condition_variable>
#include <atomic>
#include <string>
#include <iostream>
#include <nats/nats.h>
#include "BusInstance.h"  // BusInstance singleton sınıfını dahil ediyoruz

class BusJetPub {
    std::queue<std::string> messageQueue;
    std::mutex queueMutex;
    std::condition_variable cv;
    std::atomic<bool> stopFlag;
    std::thread publisherThread;
    jsCtx* js;  // JetStream context

public:
    BusJetPub() : stopFlag(false), js(nullptr) {
        // BusInstance üzerinden NATS bağlantısını alıyoruz
        const BusInstance* busInstance = BusInstance::getInstance();
        natsConnection* conn = busInstance->getConnection();

        // JetStream context oluşturuluyor
        natsConnection_JetStream(&js, conn, nullptr);
        if (js == nullptr) {
            std::cerr << "Failed to initialize JetStream context" << std::endl;
            return;
        }

        // Publisher thread başlatılıyor
        publisherThread = std::thread(&BusJetPub::processQueue, this);
    }

    ~BusJetPub() {
        stopFlag = true;
        cv.notify_all();
        if (publisherThread.joinable()) {
            publisherThread.join();
        }

        // JetStream context temizleniyor
        if (js != nullptr) {
            jsCtx_Destroy(js);
        }
    }

    // Mesajı JetStream kuyruğuna ekleyen fonksiyon
    void jetpublish(const std::string& subject, const std::string& message) {
        std::lock_guard<std::mutex> lock(queueMutex);
        messageQueue.push(subject + "|" + message);
        cv.notify_one();  // Kuyruğa mesaj eklendiğinde thread'i uyandır
    }

private:
    // Kuyruktaki mesajları JetStream ile yayınlayan fonksiyon
    void processQueue() {
        while (!stopFlag) {
            std::unique_lock<std::mutex> lock(queueMutex);
            cv.wait(lock, [this]() { return !messageQueue.empty() || stopFlag; });

            if (!messageQueue.empty()) {
                std::string message = messageQueue.front();
                messageQueue.pop();
                lock.unlock();

                std::string subject = message.substr(0, message.find("|"));
                std::string content = message.substr(message.find("|") + 1);

                jsPubAck* pubAck = nullptr;
                if (const natsStatus status = js_Publish(&pubAck, js, subject.c_str(), content.c_str(), content.size(), NULL);
                    status == NATS_OK) {
                    std::cout << "Published message: " << message << std::endl;
                } else {
                    std::cerr << "Failed to publish message: " << natsStatus_GetText(status) << std::endl;
                }
                jsPubAck_Destroy(pubAck);
            }
        }
    }
};

#endif // BUS_JET_PUB_H