//
// Created by Numan on 8.09.2024.
//

#ifndef ADMIN_H
#define ADMIN_H
//#include "../core/entity/Company.h"
#include "../infra/Bus/BusSub.h"
#include <nats/nats.h>
class AdminMan {
    BusSub busSubBoot;
public:
    explicit AdminMan() : busSubBoot("boot.*") {
        // std::function kullanarak üye fonksiyonunu BusSub'a geçiyoruz
        busSubBoot.subscribe([this](natsMsg* msg) {
            this->process_boot_messages(msg);
        });
    }


private:
    void process_boot_messages(natsMsg* msg) {
        const char* subject = natsMsg_GetSubject(msg);
        const char* data = natsMsg_GetData(msg);
        std::cout << "Processing boot "<<  subject <<" messages: " << data << std::endl;

    }

};



#endif //ADMIN_H
