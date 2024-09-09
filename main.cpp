#include <memory>
#include <string>
#include <memory>
#include <thread>

#include "src/admin/Admin.h"
#include "src/admin/data/Company.h"
#include "src/admin/data/Instrument.h"
#include "src/sys/Boot.h"
#include "src/sys/Logger.h"
#include "src/sys/NATSLogger.h"

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
