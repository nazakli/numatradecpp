//
// Created by Numan on 21.09.2024.
//

#ifndef BUSJETSUB_H
#define BUSJETSUB_H
#ifndef BUS_JET_SUB_H
#define BUS_JET_SUB_H

#include <nats/nats.h>
#include <string>
#include <iostream>
#include "BusInstance.h"  // BusInstance singleton sınıfını kullanıyoruz

class BusJetSub {
    natsConnection* conn;
    jsCtx* js;  // JetStream context
    natsSubscription* sub;
    std::string subject;
    bool isDurable;
    std::string durableName;  // Durable consumer adı

public:
    // Constructor, durable olup olmadığını ve durable consumer adını belirler
    explicit BusJetSub(std::string subject, const bool durable = false, std::string durableConsumerName = "")
        : conn(nullptr), js(nullptr), sub(nullptr), subject(std::move(subject)), isDurable(durable), durableName(std::move(durableConsumerName)) {
        // BusInstance üzerinden NATS bağlantısını alıyoruz
        BusInstance* busInstance = BusInstance::getInstance();
        conn = busInstance->getConnection();

        // JetStream context oluşturuluyor
        natsConnection_JetStream(&js, conn, NULL);
        if (js == nullptr) {
            std::cerr << "Failed to initialize JetStream context" << std::endl;
        }
    }

    // Mesajlara abone olma fonksiyonu (durable veya ephemeral)
    void subscribe(void (*callback)(natsConnection*, natsSubscription*, natsMsg*, void*), void* closure) {
        if (js != nullptr) {
            jsSubOptions* subOpts = nullptr;
            jsSubOptions_Init(&subOpts);

            if (isDurable && !durableName.empty()) {
                // Parametrik durable consumer ayarları yapılıyor
                jsSubOptions_SetDurable(subOpts, durableName.c_str());
            }

            natsStatus status = js_Subscribe(&sub, js, subject.c_str(), callback, closure, subOpts);
            if (status == NATS_OK) {
                std::cout << "Subscribed to subject: " << subject << (isDurable ? " (Durable: " + durableName + ")" : " (Ephemeral)") << std::endl;
            } else {
                std::cerr << "Failed to subscribe to subject: " << natsStatus_GetText(status) << std::endl;
            }

            // Tüketici seçeneklerini temizle
            jsSubOptions_Destroy(subOpts);
        }
    }

    // Abonelikten çıkma işlemi
    void unsubscribe() {
        if (sub) {
            natsSubscription_Unsubscribe(sub);
            natsSubscription_Destroy(sub);
            sub = nullptr;
        }
    }

    ~BusJetSub() {
        unsubscribe();
        if (js != nullptr) {
            jsCtx_Destroy(js);
        }
    }
};

#endif // BUS_JET_SUB_H

