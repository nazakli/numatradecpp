cmake_minimum_required(VERSION 3.29)
project(numatrade)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(numatrade main.cpp
        src/core/entity/Client.h
        src/core/entity/Account.h
        src/core/entity/Company.h
        src/core/entity/Exchange.h
        src/core/entity/Group.h
        src/core/entity/Instrument.h
        src/core/entity/Position.h
        src/infra/Bus/BusInstance.h
        src/infra/Bus/BusPub.h
        src/infra/Bus/BusSub.h
        src/infra/Bus/BusReq.h
        src/infra/Bus/BusRep.h
        src/app/AdminMan.h
        src/infra/Frappe/FrappeInstance.h
        src/app/NotifMan.h
        src/app/TradeMan.h
        src/app/QuoteMan.h
        src/infra/Data/Db.h
        src/infra/Ipc/Ipc.h
        src/infra/Ipc/IpcPub.h
        src/infra/Ipc/IpcSub.h
        src/infra/Ipc/IpcReq.h
        src/infra/Ipc/IpcRep.h
        src/core/DataTree.h
        src/core/entity/Entity.h
        src/infra/Data/DbSeed.h
        src/infra/Log/Log.h
        src/infra/Bus/BusJetPub.h
        src/infra/Bus/BusJetSub.h
)


find_package(cppzmq)
find_package(SQLite3)
find_package(glaze)
find_package(cnats)
find_package(spdlog)
find_package(CURL)




target_link_libraries(numatrade
        cppzmq
        CURL::libcurl
        spdlog::spdlog
        SQLite::SQLite3
        glaze::glaze
        cnats::nats_static)