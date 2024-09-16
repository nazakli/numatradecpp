#include <iostream>
#include "src/app/QuoteMan.h"
#include "src/app/TradeMan.h"

int main() {
    // QuoteMan ve TradeMan nesnelerini oluşturuyoruz
    QuoteMan quoteMan;
    TradeMan tradeMan;

    // TradeMan kotasyonları dinlemeye başlar
    tradeMan.startListening();
    tradeMan.startProcessing();

    // QuoteMan kotasyon üretimine başlar
    quoteMan.startPublishing();

    // Bir süre kotasyon üretimi ve işlenmesine devam edelim (örneğin 10 saniye)
    std::this_thread::sleep_for(std::chrono::seconds(1000));

    // TradeMan ve QuoteMan durdurulur
    quoteMan.stopPublishing();
    tradeMan.stop();

    return 0;
}
