#include <memory>
#include <string>
#include <memory>
#include <thread>

#include "src/app/AdminMan.h"
#include "src/core/entity/Company.h"
#include "src/core/entity/Instrument.h"
#include "src/infra/Boot.h"
#include "src/infra/Logger.h"
#include "src/infra/NATSLogger.h"

int main() {
    Logger::initialize();
    NATSLogger natsLogger;
    natsLogger.start();
    const std::unique_ptr<Boot> boot = std::make_unique<Boot>();
    boot->start();

    std::this_thread::sleep_for(std::chrono::seconds(2));

    natsLogger.wait();

    return 0;
}
