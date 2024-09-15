#ifndef BUS_REPLIER_H
#define BUS_REPLIER_H

#include "BusInstance.h"
#include <string>
#include <iostream>
#include <thread>
#include <atomic>

class BusReplier {
private:
    natsSubscription* sub;
    std::string subject;

public:
    // Constructor, dinlenecek subject'i alır
    BusReplier(const std::string& subject) : subject(subject), sub(nullptr) {}

    // Non-blocking istekleri dinleme ve yanıt verme fonksiyonu
    void listen(void (*callback)(natsConnection*, natsSubscription*, natsMsg*, void*)) {
        BusInstance* busInstance = BusInstance::getInstance();
        natsConnection* conn = busInstance->getConnection();

        // NATS üzerinde dinleme başlatılıyor
        natsConnection_Subscribe(&sub, conn, subject.c_str(), callback, nullptr);
        std::cout << "Listening for requests on subject: " << subject << std::endl;
    }

    // Destructor'da unsubscribe işlemi otomatik olarak yapılır
    ~BusReplier() {
        if (sub != nullptr) {
            natsSubscription_Unsubscribe(sub);
            natsSubscription_Destroy(sub);
            std::cout << "Stopped listening on subject: " << subject << std::endl;
        }
    }
};

#endif // BUS_REPLIER_H