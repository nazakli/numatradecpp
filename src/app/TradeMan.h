#ifndef TRADEMAN_H
#define TRADEMAN_H

#include "../infra/Bus/BusSubscriber.h"
#include <atomic>
#include <thread>
#include <iostream>
#include <queue>
#include <mutex>
#include <condition_variable>

#include "../infra/Ipc/IpcSub.h"

class TradeMan {
private:
  BusSubscriber subscriber; // BusSubscriber private alan
  IpcSub ipcSub; // IpcSub private alan
  std::atomic<bool> stopFlag;
  std::thread subscriberThread;
  std::thread processorThread;
  std::queue<std::pair<std::string, std::string> > quoteQueue; // Kotasyonları tutan kuyruk
  std::mutex queueMutex;
  std::condition_variable cv; // Condition variable

public:
  // Constructor, parametre almıyor
  TradeMan() : subscriber("quotes.*"), ipcSub("quotes", [this](const std::string &message) {
    this->ipcListen(message); // Fiyat geldiğinde callback'i çalıştır
  }), stopFlag(false) {
  }

  // Dinlemeyi başlatan fonksiyon
  void ipcListen(const std::string &message) {
    std::cout << "Received IPC message: " << message << std::endl;
    {
      std::lock_guard<std::mutex> lock(queueMutex);
      quoteQueue.emplace("quotes", message);
    }
      cv.notify_one();
  };


  // Kotasyonları kuyruğa ekleme fonksiyonu
  void enqueueQuote(const std::string &subject, const std::string &content) { {
      std::lock_guard<std::mutex> lock(queueMutex);
      quoteQueue.emplace(subject, content);
      // std::cout << "Enqueued quote: " << subject << " - " << content << std::endl;  // Kotasyon geldiğinde log
    }
    // Yeni kotasyon eklendiğinde condition variable tetikleniyor
    cv.notify_one();
  }

  // Üye fonksiyon olarak message handler
  void messageHandler(natsConnection *conn, natsSubscription *sub, natsMsg *msg) {
    const std::string subject(natsMsg_GetSubject(msg));
    const std::string content(natsMsg_GetData(msg), natsMsg_GetDataLength(msg));

    // std::cout << "Received quote: " << subject << " - " << content << std::endl;  // Mesaj geldiğinde log

    // Kuyruğa kotasyon ekle
    enqueueQuote(subject, content);

    natsMsg_Destroy(msg); // Mesajı temizle
  }

  // Kotasyonları dinleme ve kuyruğa ekleme fonksiyonu
  void startListening() {
    subscriberThread = std::thread([this]() {
      // Dinleme başlatılıyor
      subscriber.subscribe(
        [](natsConnection *conn, natsSubscription *sub, natsMsg *msg, void *closure) {
          TradeMan *tradeMan = static_cast<TradeMan *>(closure);
          tradeMan->messageHandler(conn, sub, msg);
        },
        this // Closure olarak 'this' kullanılıyor
      );

      std::cout << "TradeMan is now listening for quotes..." << std::endl; // Listening başladı logu

      // Bu thread sadece listener, stopFlag kullanılarak sonlandırılır
      while (!stopFlag.load()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100)); // CPU tüketimini azaltmak için
      }
    });
  }

  // Kuyruktaki mesajları işleme fonksiyonu (non-blocking)
  void startProcessing() {
    processorThread = std::thread([this]() {
      while (!stopFlag.load()) {
        std::unique_lock<std::mutex> lock(queueMutex);

        // Kotasyon kuyruğunda bir şey olduğunda uyanmak için condition variable kullanıyoruz
        cv.wait(lock, [this] { return !quoteQueue.empty() || stopFlag.load(); });

        // Kuyruktaki tüm mesajları işleyelim
        while (!quoteQueue.empty()) {
          auto quote = quoteQueue.front();
          quoteQueue.pop();

          // İşleme (burada mesajı ekrana yazıyoruz)
          std::cout << "Processed quote: " << quote.first << " - " << quote.second << std::endl;
        }
      }
    });
  }

  // Dinlemeyi durdurma fonksiyonu
  void stop() {
    stopFlag = true;
    cv.notify_all(); // Kuyruktaki işlemler için sinyal gönder
    if (subscriberThread.joinable()) {
      subscriberThread.join();
    }
    if (processorThread.joinable()) {
      processorThread.join();
    }
    subscriber.unsubscribe(); // Unsubscribe işlemi yapılır
  }

  ~TradeMan() {
    stop();
  }
};

#endif // TRADEMAN_H
