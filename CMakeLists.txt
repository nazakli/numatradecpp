cmake_minimum_required(VERSION 3.29)
project(numatrade)

set(CMAKE_CXX_STANDARD 23)

add_executable(numatrade main.cpp
        src/core/entity/Company.h
        src/core/entity/Company.h
        src/app/AdminMan.h
        src/core/enum/InstrumentType.h
        src/core/entity/Instrument.h
        src/infra/Bus.h
        src/infra/NatsBase.h
        src/infra/boot.h
        src/infra/Logger.h
        src/infra/NATSLogger.h
        src/infra/Ipc.h
        src/core/entity/Client.h
        src/app/AccountMan.h
        src/core/entity/Group.h
        src/core/entity/Account.h
        src/core/entity/Position.h
        src/app/BrokerMan.h
        src/app/QuoteMan.h
        src/app/MoneyMan.h
        src/app/RiskMan.h
        src/app/TradeMan.h
        src/app/OnlineMan.h
        src/app/NotifMan.h
        src/app/ReportMan.h
        src/app/EmailMan.h
        src/app/NewsMan.h
        src/core/entity/Exchange.h

)
find_package(nng)
find_package(spdlog)
find_package(glaze)
find_package(cnats)

target_link_libraries(numatrade
        nng::nng
        spdlog::spdlog
        glaze::glaze
        cnats::nats_static)