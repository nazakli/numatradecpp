cmake_minimum_required(VERSION 3.29)
project(numatrade)

set(CMAKE_CXX_STANDARD 23)

add_executable(numatrade main.cpp
        src/core/entity/Client.h
        src/core/entity/Account.h
        src/core/entity/Company.h
        src/core/entity/Exchange.h
        src/core/entity/Group.h
        src/core/entity/Instrument.h
        src/core/entity/Position.h
        src/infra/Bus/BusInstance.h
        src/infra/Bus/BusPublisher.h
        src/infra/Bus/BusSubscriber.h
        src/infra/Bus/BusRequester.h
        src/infra/Bus/BusReplier.h
        src/app/AdminMan.h
)
find_package(nng)
find_package(spdlog)
find_package(glaze)
find_package(cnats)
find_package(SQLite3)


target_link_libraries(numatrade
        nng::nng
        spdlog::spdlog
        SQLite::SQLite3
        glaze::glaze
        cnats::nats_static)