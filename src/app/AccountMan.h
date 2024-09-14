//
// Created by Numan on 14.09.2024.
//

#ifndef ACCOUNTMAN_H
#define ACCOUNTMAN_H
#include "../infra/NatsBase.h"

class AccountMan : NatsBase {
public:
    subIpc(IpcMsg msg) {
        std::cout << "AccountMan::subIpc: " << msg << std::endl;
    }

    AccountMan() {
        pubBus("account", "Account is created.");
        IpcMsg msg("INFO", "{'data': 'AccountMan is created.'}");
        bool stat = pubIpc(msg);
    }
    ~AccountMan() {
        pubBus("account", "Account is destroyed.");
        IpcMsg msg("INFO", "{'data': 'AccountMan is destroyed.'}");
        bool stat = pubIpc(msg);
    }
};
#endif //ACCOUNTMAN_H
