#include <iostream>
#include "src/app/QuoteMan.h"
#include "src/app/TradeMan.h"


int main() {

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
