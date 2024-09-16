//
// Created by Numan on 16.09.2024.
//

#ifndef IPCSUB_H
#define IPCSUB_H
#include <functional>
#include <string>
#include <utility>
#include <zmq.hpp>
#include "Ipc.h"

class IpcSub {
public:
  // Constructor: ZeroMQ context'i singleton'dan alıyor ve dinamik kanala abone oluyor
  IpcSub(const std::string& channel, std::function<void(const std::string&)> callback)
      : socket(Ipc::getInstance(), ZMQ_SUB), callback(std::move(callback)) {
    // inproc üzerinden "ipc_pubsub" adresine bağlan
    socket.connect("inproc://ipc_pubsub");

    // Belirli bir kanala abone ol (dinamik kanal ismi)
    socket.set(zmq::sockopt::subscribe, channel);
  }

  // Mesaj dinleme ve callback'i çalıştırma (CPU dostu blocking mod)
  void listen() {
    zmq::pollitem_t items[] = { { static_cast<void*>(socket), 0, ZMQ_POLLIN, 0 } };

    while (true) {
      // CPU dostu blocking poll - süresiz bekleme
      zmq::poll(items, 1, -1);  // -1 ile süresiz bekle, mesaj gelir gelmez tetiklenir

      // Eğer bir mesaj varsa
      if (items[0].revents & ZMQ_POLLIN) {
        zmq::message_t message;
        socket.recv(message, zmq::recv_flags::none);  // Mesajı al
        std::string receivedMessage(static_cast<char*>(message.data()), message.size());

        // Callback fonksiyonunu çalıştır
        callback(receivedMessage);
      }
    }
  }

private:
  zmq::socket_t socket;    // ZeroMQ subscriber socket
  std::function<void(const std::string&)> callback; // Dinamik callback fonksiyonu
};
#endif //IPCSUB_H
