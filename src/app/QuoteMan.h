#ifndef QUOTEMAN_H
#define QUOTEMAN_H

#include "../infra/Bus/BusPublisher.h"
#include <thread>
#include <atomic>
#include <random>
#include <vector>
#include <string>
#include <chrono>
#include <map>
#include <glaze/glaze.hpp> // Glaze kütüphanesi için

#include "../infra/Data/Db.h"
#include "../infra/Ipc/IpcPub.h"

// Glaze kullanarak bid/ask yapısı için JSON formatını tanımlıyoruz
struct Quote {
  std::string s;
  double b;
  double a;
  uint64_t t; // Nanosaniye cinsinden timestamp

  // Glaze serileştirme için yapı tanımı
  template<auto & Buffer>
  struct glaze {
    using T = Quote;
    static constexpr auto value = glz::object(
      "s", &T::s,
      "b", &T::b,
      "a", &T::a,
      "t", &T::t // Timestamp alanı JSON formatına ekleniyor
    );
  };
};

class QuoteMan {
private:
  BusPublisher publisher; // BusPublisher private alan
  IpcPub ipcPub; // IpcPub private alan
  std::atomic<bool> stopFlag;
  std::thread quoteThread;
  std::vector<std::string> symbols;
  std::map<std::string, double> initialPrices; // Başlangıç fiyatları
  std::default_random_engine generator;
  std::uniform_real_distribution<double> changeDistribution;
  Db &db = Db::getInstance(); // Db instance
public:
  // Constructor
  QuoteMan()
    : stopFlag(false),
      symbols({"EURUSD", "AUDCAD", "AAPL", "BTC"}),
      changeDistribution(-0.05, 0.05) // ±%5 değişim aralığı
  {
    // Ürünlerin başlangıç fiyatlarını belirleyelim
    initialPrices["EURUSD"] = 1.2; // EUR/USD paritesi
    initialPrices["AUDCAD"] = 0.9; // AUD/CAD paritesi
    initialPrices["AAPL"] = 150.0; // AAPL hisse fiyatı
    initialPrices["BTC"] = 45000.0; // BTC fiyatı

    db.connect(); // Veritabanı bağlantısını başlat
  }

  // Rastgele bid/ask fiyatları üreten ve yayınlayan fonksiyon
  void startPublishing() {
    quoteThread = std::thread([this]() {
      while (!stopFlag.load()) {
        for (int i = 0; i < 100; ++i) {
          // Her bir sembol için rastgele bid ve ask fiyat üret
          for (const auto &symbol: symbols) {
            const double priceChange = changeDistribution(generator); // ±%5 rastgele değişim
            const double midPrice = initialPrices[symbol] * (1.0 + priceChange);
            const double bid = midPrice * 0.999; // Bid fiyatı mid price'dan düşük
            const double ask = midPrice * 1.001; // Ask fiyatı mid price'dan yüksek

            // Timestamp nanosaniye cinsinden
            const uint64_t timestamp = std::chrono::duration_cast<std::chrono::nanoseconds>(
              std::chrono::high_resolution_clock::now().time_since_epoch()).count();

            // Quote yapısı oluşturuluyor
            Quote quote{symbol, bid, ask, timestamp};

            // Glaze ile JSON formatına serileştirme

            // JSON serileştirme başarılı mı kontrol ediyoruz
            if (auto jsonQuoteResult = glz::write_json(quote); jsonQuoteResult.has_value()) {
              const std::string &jsonQuote = jsonQuoteResult.value();
              auto res = db.executeSQL(
                "INSERT OR REPLACE INTO quotes (instrument, bid, ask, timestamp) VALUES ('" + symbol +
                "', "
                + std::to_string(bid) + ", " + std::to_string(ask) + ", " + std::to_string(timestamp) +
                ");");
              // BusPublisher üzerinden JSON olarak yayınla
              ipcPub.publish("quotes",jsonQuote);
              //publisher.publish("quotes." + symbol, jsonQuote);
            } else {
              // Hata durumunda log basıyoruz
              std::cerr << "JSON serileştirme hatası: " << jsonQuoteResult.error() << std::endl;
            }
          }
          std::this_thread::sleep_for(std::chrono::microseconds(10000)); // 100 mesaj/saniye
        }
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
