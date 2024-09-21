#include <iostream>

#include "src/app/AdminMan.h"
#include "src/app/QuoteMan.h"
#include "src/app/TradeMan.h"
#include "src/infra/Bus/BusSub.h"
#include "src/infra/Data/DbSeed.h"
#include "src/infra/Frappe/FrappeInstance.h"


int main() {


    DbSeed::createTables();

    auto adminMan = std::make_unique<AdminMan>();

    // Mesajlar geldikçe işlenmesi için bir süre bekliyoruz
    std::this_thread::sleep_for(std::chrono::seconds(1));


    FrappeInstance frappeInstance("http://numa.localhost:8000", "9ae06d2bbe2e5a7", "5021f1295f661f2");

    const auto getResult = frappeInstance.asyncGet("boot.companies").get();
    std::cout << "GET Result: " << getResult << std::endl;

    // Aboneliği sonlandırıyoruz

    // // Asenkron POST isteği
    // std::string jsonData = R"({"name": "New Client", "client_type": "Individual"})";
    // auto postResult = frappeInstance.asyncPost("api/resource/Client", jsonData).get();
    // std::cout << "POST Result: " << postResult << std::endl;


    /*
    const int messageCount = 125000;  // Gönderilecek mesaj sayısı
    std::atomic<int> publishedCount = 0;  // Gönderilen mesajların sayısı
    std::atomic<int> receivedCount = 0;  // Alınan mesajların sayısı

    QuoteMan quoteMan;
    TradeMan tradeMan;



    // TradeMan önce mesajları dinlemeye başlasın
    std::thread subscriberThread([&]() {
        tradeMan.startProcessing(messageCount, receivedCount);
    });

    // QuoteMan mesajları göndermeye başlasın
    std::thread publisherThread([&]() {
        quoteMan.startPublishing(messageCount, publishedCount);
    });

    // Thread'lerin bitmesini bekle
    publisherThread.join();
    subscriberThread.join();
    */


    std::this_thread::sleep_for(std::chrono::seconds(1000));

    // TradeMan ve QuoteMan durdurulur
    //quoteMan.stopPublishing();
    //tradeMan.stop();

    return 0;
}
