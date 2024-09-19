#ifndef QUOTEMAN_H
#define QUOTEMAN_H

#include <thread>
#include <atomic>
#include <random>
#include <vector>
#include <string>
#include <chrono>
#include <map>
#include <glaze/glaze.hpp>  // Glaze kütüphanesi için
#include "../infra/Ipc/IpcPub.h"

// Glaze kullanarak bid/ask yapısı için JSON formatını tanımlıyoruz
struct Quote {
  std::string s;  // Sembol
  double b;       // Bid fiyatı
  double a;       // Ask fiyatı
  uint64_t t;     // Nanosaniye cinsinden timestamp

  // Glaze serileştirme için yapı tanımı
  template<auto & Buffer>
  struct glaze {
    using T = Quote;
    static constexpr auto value = glz::object(
      "s", &T::s,
      "b", &T::b,
      "a", &T::a,
      "t", &T::t  // Timestamp alanı JSON formatına ekleniyor
    );
  };
};

class QuoteMan {
private:
  IpcPub ipcPub;  // IpcPub private alan
  std::atomic<bool> stopFlag;
  std::thread quoteThread;
  std::vector<std::string> symbols;
  std::map<std::string, double> initialPrices;  // Başlangıç fiyatları
  std::default_random_engine generator;
  std::uniform_real_distribution<double> changeDistribution;

public:
  // Constructor
  QuoteMan()
    : stopFlag(false),
      symbols({"EURUSD", "AUDCAD", "AAPL", "BTC"}),
      changeDistribution(-0.05, 0.05)  // ±%5 değişim aralığı
  {
    // Ürünlerin başlangıç fiyatlarını belirleyelim
    initialPrices["EURUSD"] = 1.2;  // EUR/USD paritesi
    initialPrices["AUDCAD"] = 0.9;  // AUD/CAD paritesi
    initialPrices["AAPL"] = 150.0;  // AAPL hisse fiyatı
    initialPrices["BTC"] = 45000.0;  // BTC fiyatı
  }



  /*
  void startPublishing(const int messageCount, std::atomic<int>& publishedCount) {
    IpcPub ipcPub;
    std::vector<std::string> symbols = {"EURUSD", "AUDCAD", "AAPL", "BTC"};
    // std::vector<std::string> symbols = {"EURUSD", "AAPL", "BTC"};
    std::default_random_engine generator;
    std::uniform_real_distribution<double> distribution(-0.05, 0.05);  // ±5% değişim

    const auto startTime = std::chrono::high_resolution_clock::now();  // Başlangıç zamanı

    for (int i = 0; i < messageCount; ++i) {
      for (const auto& symbol : symbols) {
        const double midPrice = 100.0 * (1.0 + distribution(generator));  // Rastgele fiyat
        const double bid = midPrice * 0.999;
        const double ask = midPrice * 1.001;

        // Zaman damgası nanosaniye cinsinden
        const uint64_t timestamp = std::chrono::duration_cast<std::chrono::nanoseconds>(
            std::chrono::high_resolution_clock::now().time_since_epoch()).count();

        // Quote yapısı JSON serileştirme için
        Quote quote{symbol, bid, ask, timestamp};
        if (auto jsonResult = glz::write_json(quote); jsonResult.has_value()) {
          ipcPub.publish("quotes", jsonResult.value());  // JSON mesaj gönder
          ++publishedCount;  // Gönderilen mesaj sayısını artır
        }
      }
      std::this_thread::sleep_for(std::chrono::microseconds(1));  // 100 ms bekle
    }

    const auto endTime = std::chrono::high_resolution_clock::now();  // Bitiş zamanı
    const std::chrono::duration<double> elapsed = endTime - startTime;

    std::cout << "Published " << publishedCount.load() << " JSON fiyat mesajı in "
              << elapsed.count() << " seconds." << std::endl;
  }
  */

  // Rastgele bid/ask fiyatları üreten ve yayınlayan fonksiyon
  void startPublishing() {
    quoteThread = std::thread([this]() {
      while (!stopFlag.load()) {
        for (const auto &symbol : symbols) {
          const double priceChange = changeDistribution(generator);  // ±%5 rastgele değişim
          const double midPrice = initialPrices[symbol] * (1.0 + priceChange);
          const double bid = midPrice * 0.999;  // Bid fiyatı mid price'dan düşük
          const double ask = midPrice * 1.001;  // Ask fiyatı mid price'dan yüksek

          // Timestamp nanosaniye cinsinden
          const uint64_t timestamp = std::chrono::duration_cast<std::chrono::nanoseconds>(
            std::chrono::high_resolution_clock::now().time_since_epoch()).count();

          // Quote yapısı oluşturuluyor
          Quote quote{symbol, bid, ask, timestamp};

          // Glaze ile JSON formatına serileştirme
          if (auto jsonQuoteResult = glz::write_json(quote); jsonQuoteResult.has_value()) {
            const std::string &jsonQuote = jsonQuoteResult.value();
            ipcPub.publish("quotes", jsonQuote);  // "quotes" kanalına JSON mesaj gönder
            std::cout << "Published quote: " << jsonQuote << std::endl;
          } else {
            std::cerr << "JSON serileştirme hatası" << std::endl;
          }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));  // 1 saniyede bir mesaj
      }
    });
  }

  // Yayını durdurma fonksiyonu
  void stopPublishing() {
    stopFlag = true;
    if (quoteThread.joinable()) {
      quoteThread.join();
    }
  }

  ~QuoteMan() {
    stopPublishing();
  }
};

#endif // QUOTEMAN_H