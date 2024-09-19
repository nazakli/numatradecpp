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
#include <thread>
#include <iostream>

class IpcSub {
public:
  IpcSub(const std::string& channel, std::function<void(const std::string&)> callback)
      : socket(Ipc::getInstance(), ZMQ_SUB), callback(std::move(callback)) {
    socket.connect("inproc://ipc_pubsub");  // Kanal ismi ile bağlan
    socket.set(zmq::sockopt::subscribe, channel);  // Kanal abonesi ol
  }

  // Dinleme fonksiyonunu başlat
  void listen() {
    std::thread([this]() {
        zmq::pollitem_t items[] = { { static_cast<void*>(socket), 0, ZMQ_POLLIN, 0 } };

        // Süresiz mesaj dinleme döngüsü
        while (true) {
            zmq::poll(items,1, std::chrono::milliseconds(-1));  // Mesaj gelene kadar bekler

            // Mesaj geldiğinde
            if (items[0].revents & ZMQ_POLLIN) {
                zmq::message_t message;
                socket.recv(message, zmq::recv_flags::none);  // Mesajı al
                std::string msg(static_cast<char*>(message.data()), message.size());
                callback(msg);  // Callback fonksiyonunu çalıştır
            }
        }
    }).detach();  // Thread'i detach ederek çalıştır
  }

private:
  zmq::socket_t socket;  // ZeroMQ socket
  std::function<void(const std::string&)> callback;  // Mesaj geldiğinde çalışacak callback
};
#endif //IPCSUB_H
