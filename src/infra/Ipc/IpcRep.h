//
// Created by Numan on 17.09.2024.
//

#ifndef IPCREP_H
#define IPCREP_H
#include <functional>
#include <iostream>
#include <string>
#include <thread>
#include <utility>
#include <zmq.hpp>

#include "Ipc.h"

class IpcRep {
public:
    // Constructor: ZeroMQ context'i singleton'dan alıyor ve REP soketi bağlanıyor
    explicit IpcRep(std::function<void(const std::string&, std::function<void(const std::string&)>)> callback)
        : socket(Ipc::getInstance(), ZMQ_REP), callback(std::move(callback)) {
        // inproc üzerinden "ipc_pubsub" adresine bağlan
        socket.bind("inproc://ipc_pubsub");
    }

    // İstekleri dinleyip, yanıtları callback ile işlemek için asenkron dinleme
    void listen() {
        std::thread([this]() {
            zmq::pollitem_t items[] = { { static_cast<void*>(socket), 0, ZMQ_POLLIN, 0 } };

            while (true) {
                // Non-blocking modda istekleri dinle (yanıt gelene kadar bloklama yok)
                zmq::poll(items, 1, -1);  // -1 ile süresiz bekle

                if (items[0].revents & ZMQ_POLLIN) {
                    zmq::message_t request;
                    socket.recv(request, zmq::recv_flags::none);  // İstek mesajını al
                    std::string requestMessage(static_cast<char*>(request.data()), request.size());

                    // Yanıtı geri göndermek için callback fonksiyonunu çalıştır
                    callback(requestMessage, [this](const std::string& replyMessage) {
                        // Yanıt gönderme işlemini ayrı bir thread'de çalıştır
                        std::thread([this, replyMessage]() {
                            zmq::message_t zmq_reply(replyMessage.size());
                            memcpy(zmq_reply.data(), replyMessage.data(), replyMessage.size());

                            // Yanıtı non-blocking modda gönder
                            if (const zmq::send_result_t sent = socket.send(zmq_reply, zmq::send_flags::dontwait)) {
                                std::cout << "Yanıt başarıyla gönderildi: " << replyMessage << std::endl;
                            } else {
                                std::cerr << "Yanıt gönderilemedi (non-blocking modda)." << std::endl;
                            }
                        }).detach();  // Yanıt gönderme işlemi bağımsız thread'de çalışır
                    });
                }
            }
        }).detach();  // Thread'i detach ederek bağımsız çalışmasını sağla
    }

private:
    zmq::socket_t socket;  // ZeroMQ reply socket (REP)
    std::function<void(const std::string&, std::function<void(const std::string&)>)> callback;  // Dinamik callback fonksiyonu
};
#endif //IPCREP_H
