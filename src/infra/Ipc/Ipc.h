//
// Created by Numan on 16.09.2024.
//

#ifndef IPC_H
#define IPC_H
#include <zmq.hpp>

// Singleton ZeroMQ Context sınıfı
class Ipc {
public:
  // Ipc sınıfının tek örneğini alır
  static   zmq::context_t& getInstance() {
    static zmq::context_t instance(3); // 3 thread'li context
    return instance;
  }

  // Kopyalanabilir ve taşınabilir olmasını engelle
  Ipc(const Ipc&) = delete;
  Ipc& operator=(const Ipc&) = delete;

private:
  Ipc() = default; // Özel yapıcı
};
#endif //IPC_H
