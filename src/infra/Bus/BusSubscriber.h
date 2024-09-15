//
// Created by Numan on 15.09.2024.
//

#ifndef BUS_SUBSCRIBER_H
#define BUS_SUBSCRIBER_H

#include "BusInstance.h"
#include <string>
#include <iostream>
#include <thread>
#include <atomic>

class BusSubscriber {
private:
    natsSubscription* sub;
    std::string subject;
    std::atomic<bool> stopFlag;
    std::thread subscriberThread;

public:
    // Constructor, abone olunacak kanalı alır
    BusSubscriber(const std::string& subject) : subject(subject), sub(nullptr), stopFlag(false) {}

    // Abonelik fonksiyonu (non-blocking, ayrı thread)
    void subscribe(void (*callback)(natsConnection*, natsSubscription*, natsMsg*, void*)) {
        BusInstance* busInstance = BusInstance::getInstance();
        natsConnection* conn = busInstance->getConnection();

        natsConnection_Subscribe(&sub, conn, subject.c_str(), callback, nullptr);
        std::cout << "Subscribed to subject: " << subject << std::endl;

        // Mesajların dinlenmesi için ayrı bir thread başlatılır
        subscriberThread = std::thread([this]() {
            while (!stopFlag.load()) {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));  // CPU tüketimini düşürmek için kısa bir bekleme
            }
        });
    }

    // Destructor'da unsubscribe işlemi otomatik yapılır
    ~BusSubscriber() {
        stopFlag = true;  // Durdurmak için flag'i ayarla
        if (subscriberThread.joinable()) {
            subscriberThread.join();  // Thread'i düzgün bir şekilde bitir
        }

        if (sub != nullptr) {
            natsSubscription_Unsubscribe(sub);  // Abonelik iptal edilir
            natsSubscription_Destroy(sub);  // Subscription temizlenir
            std::cout << "Unsubscribed and cleaned up subject: " << subject << std::endl;
        }
    }

    // Manuel olarak unsubscribe etmeye gerek yok, destructor bunu yapar
};

#endif // BUS_SUBSCRIBER_H