#ifndef TRADEMAN_H
#define TRADEMAN_H

#include <atomic>
#include <condition_variable>
#include <iostream>
#include <queue>
#include <thread>
#include "../infra/Ipc/IpcSub.h"

class TradeMan {
  IpcSub ipcSub;  // IpcSub private alan
  std::atomic<bool> stopFlag;
  std::thread processorThread;
  std::queue<std::string> quoteQueue;  // Kuyruğa gelen kotasyonlar eklenir
  std::mutex queueMutex;
  std::condition_variable cv;  // Kuyruk işlemleri için condition variable

public:
  // Constructor

  TradeMan()
    : ipcSub("quotes", [this](const std::string &message) {
        this->ipcListen(message);  // Fiyat geldiğinde callback'i çalıştır
      }),
      stopFlag(false) {
    ipcSub.listen();
  }


  // Fiyatları kuyruğa ekleyen fonksiyon
  void ipcListen(const std::string &message) {
    //std::cout << "Received quote: " << message << "" << std::endl;
    {
      std::lock_guard<std::mutex> lock(queueMutex);
      quoteQueue.push(message);
    }
    cv.notify_one();  // Kuyruğa yeni mesaj geldiğinde bildirim yap
  }


  /*void startProcessing(const int expectedMessages, std::atomic<int>& receivedCount) {
    const auto startTime = std::chrono::high_resolution_clock::now();  // Başlangıç zamanı

    IpcSub ipcSub("quotes", [&](const std::string& message) {
        ++receivedCount;  // Alınan mesaj sayısını artır
        std::cout << "Received JSON: " << message << std::endl;  // JSON mesajı göster

        if (receivedCount == expectedMessages) {
          // Bitiş zamanı
          const std::chrono::time_point<std::chrono::steady_clock> endTime = std::chrono::high_resolution_clock::now();
          const std::chrono::duration<double> elapsed = endTime - startTime;

            std::cout << "Received " << receivedCount.load() << " JSON mesajı in "
                      << elapsed.count() << " seconds." << std::endl;

            // Saniyede alınan mesaj sayısını hesapla
          const double messagesPerSecond = receivedCount / elapsed.count();
            std::cout << "Messages per second: " << messagesPerSecond << std::endl;
        }
    });
  }*/

  // Kuyruğu işleyen thread'i başlatır
  void startProcessing() {
    processorThread = std::thread([this]() {
      while (!stopFlag.load()) {
        std::unique_lock<std::mutex> lock(queueMutex);

        // Kuyrukta mesaj olduğunda uyanmak için condition variable kullanıyoruz
        cv.wait(lock, [this] { return !quoteQueue.empty() || stopFlag.load(); });

        // Kuyruktaki tüm mesajları işleyelim
        while (!quoteQueue.empty()) {
          std::string quoteMessage = quoteQueue.front();
          quoteQueue.pop();

          // İşleme (mesajı ekrana yazıyoruz)
          std::cout << "Processed quote: " << quoteMessage << std::endl;
        }
      }
    });
  }

  // Dinlemeyi durdurma fonksiyonu
  void stop() {
    stopFlag = true;
    cv.notify_all();  // Kuyruktaki işlemler için sinyal gönder
    if (processorThread.joinable()) {
      processorThread.join();
    }
  }

  ~TradeMan() {
    stop();  // Sınıf yok edilirken thread'i güvenli bir şekilde durdur
  }
};

#endif // TRADEMAN_H