#ifndef BUS_SUBSCRIBER_H
#define BUS_SUBSCRIBER_H

#include <nats/nats.h>
#include <string>
#include <iostream>
#include <functional> // std::function

#include "BusInstance.h"

class BusSub {
  natsConnection* conn;
  natsSubscription* sub;
  std::string subject;
  std::function<void(natsMsg*)> messageCallback;  // Üye fonksiyon callback

public:
  explicit BusSub(std::string subject) : conn(nullptr), sub(nullptr), subject(std::move(subject)) {
    const BusInstance* busInstance = BusInstance::getInstance();
    conn = busInstance->getConnection();
  }

  // Callback fonksiyonunu std::function ile alıyoruz
  void subscribe(const std::function<void(natsMsg*)>& callback) {
    this->messageCallback = callback;

    const natsStatus s = natsConnection_Subscribe(&sub, conn, subject.c_str(),
    [](natsConnection*, natsSubscription*, natsMsg* msg, void* closure) {
        // closure'dan std::function'a erişip callback'i çağırıyoruz
        auto* cb = static_cast<std::function<void(natsMsg*)>*>(closure);
        (*cb)(msg);  // std::function callback çağrılıyor
    }, &messageCallback);  // Closure olarak std::function'ı geçiriyoruz

    if (s != NATS_OK) {
      std::cerr << "Failed to subscribe: " << natsStatus_GetText(s) << std::endl;
    }
  }

  void unsubscribe() {
    if (sub) {
      natsSubscription_Unsubscribe(sub);
      natsSubscription_Destroy(sub);
      sub = nullptr;
    }
  }

  ~BusSub() {
    unsubscribe();
  }
};

#endif // BUS_SUBSCRIBER_H