#ifndef IPC_H
#define IPC_H

#include <zmq.hpp>

class Ipc {
public:
  // Singleton context oluşturucu
  static zmq::context_t& getInstance() {
    static zmq::context_t context(1);  // Tek thread'li ZeroMQ context
    return context;
  }

private:
  // Constructor private yapılır, böylece dışarıdan instance oluşturulamaz
  Ipc() = default;
  ~Ipc() = default;

  // Copy constructor ve assignment operator devre dışı bırakılır
  Ipc(const Ipc&) = delete;
  Ipc& operator=(const Ipc&) = delete;
};

#endif // IPC_H