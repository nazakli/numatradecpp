//
// Created by Numan on 16.09.2024.
//

#ifndef IPCREQ_H
#define IPCREQ_H
#include <utility>
#include <zmq.hpp>
#include <iostream>
#include <string>
#include <functional>
#include <thread>

#include "Ipc.h"

class IpcReq {
public:
    // Constructor: ZeroMQ context'i singleton'dan alıyor ve soketi bağlanıyor
    explicit IpcReq(std::function<void(const std::string&)> callback)
        : socket(Ipc::getInstance(), ZMQ_REQ), callback(std::move(callback)) {
        // inproc üzerinden "ipc_pubsub" adresine bağlan
        socket.connect("inproc://ipc_pubsub");
    }

    // Non-blocking olarak istek gönder
    void request(const std::string& message) {
        zmq::message_t zmq_message(message.size());
        memcpy(zmq_message.data(), message.data(), message.size());

        // Non-blocking modda istek gönder
        if (const zmq::send_result_t sent = socket.send(zmq_message, zmq::send_flags::dontwait)) {
            std::cout << "İstek başarıyla gönderildi: " << message << std::endl;
        } else {
            std::cerr << "İstek gönderilemedi, non-blocking modda." << std::endl;
        }
    }

    // Yanıtları asenkron olarak dinlemek için ayrı bir thread kullan
    void listenForReply() {
        std::thread([this]() {
            zmq::pollitem_t items[] = { { static_cast<void*>(socket), 0, ZMQ_POLLIN, 0 } };

            // Sürekli yanıtları dinle (timeout'suz, yanıt gelene kadar süresiz bekler)
            while (true) {
                zmq::poll(items, 1, -1);  // -1 ile süresiz bekle, yanıt gelir gelmez tetiklenir

                // Eğer bir yanıt varsa
                if (items[0].revents & ZMQ_POLLIN) {
                    zmq::message_t reply;
                    socket.recv(reply, zmq::recv_flags::none);  // Mesajı al
                    std::string replyMessage(static_cast<char*>(reply.data()), reply.size());

                    // Callback fonksiyonunu çalıştır
                    callback(replyMessage);
                    break;  // Yanıt alındıktan sonra döngüden çıkabiliriz
                }
            }
        }).detach();  // Thread'i detach ederek bağımsız çalışmasını sağla
    }

private:
    zmq::socket_t socket;  // ZeroMQ request socket (REQ)
    std::function<void(const std::string&)> callback; // Dinamik callback fonksiyonu
};
#endif //IPCREQ_H
