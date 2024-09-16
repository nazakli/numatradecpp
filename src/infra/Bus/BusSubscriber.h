#ifndef BUS_SUBSCRIBER_H
#define BUS_SUBSCRIBER_H

#include <nats/nats.h>
#include <string>
#include <iostream>

class BusSubscriber {
private:
    natsConnection* conn;
    natsSubscription* sub;
    std::string subject;

public:
    BusSubscriber(const std::string& subject) : conn(nullptr), sub(nullptr), subject(subject) {
        // NATS bağlantısı başlatılıyor
        natsConnection_ConnectTo(&conn, NATS_DEFAULT_URL);
    }

    // Closure ile çalışan abonelik başlatma fonksiyonu
    void subscribe(void (*callback)(natsConnection*, natsSubscription*, natsMsg*, void*), void* closure) {
        natsConnection_Subscribe(&sub, conn, subject.c_str(), callback, closure);
    }

    // Unsubscribe işlemi
    void unsubscribe() {
        if (sub) {
            natsSubscription_Unsubscribe(sub);
            natsSubscription_Destroy(sub);
            sub = nullptr;
        }
    }

    ~BusSubscriber() {
        unsubscribe();
        if (conn) {
            natsConnection_Close(conn);
            natsConnection_Destroy(conn);
        }
    }
};

#endif // BUS_SUBSCRIBER_H