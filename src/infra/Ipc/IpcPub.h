//
// Created by Numan on 16.09.2024.
//

#ifndef IPCPUB_H
#define IPCPUB_H

#include <zmq.hpp>
#include <string>
#include <iostream>
#include "Ipc.h"  // Singleton sınıfı burada kullanılıyor

class IpcPub {
public:
  // Constructor: ZeroMQ context'i singleton'dan alıyor ve soketi "ipc_pubsub" adresine bağlıyor
  IpcPub() : socket(Ipc::getInstance(), ZMQ_PUB) {
    // inproc üzerinden "ipc_pubsub" adresine bağlan
    socket.bind("inproc://ipc_pubsub");
  }

  // Mesajı non-blocking olarak yayınla
  void publish(const std::string& channel, const std::string& message) {
    const std::string fullMessage = channel + " " + message;  // Dinamik kanal ismi ekle
    zmq::message_t zmq_message(fullMessage.size());
    memcpy(zmq_message.data(), fullMessage.data(), fullMessage.size());

    // Mesajı non-blocking modda gönder (dontwait ile)
    if (const zmq::send_result_t sent = socket.send(zmq_message, zmq::send_flags::dontwait)) {
      //std::cout << "IPC PUB [" << channel << "]: " << message << std::endl;
    } else {
      std::cerr << "Mesaj gönderilemedi (non-blocking modda)." << std::endl;
    }
  }

private:
  zmq::socket_t socket;    // ZeroMQ publisher socket
};

#endif //IPCPUB_H
