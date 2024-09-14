cmake_minimum_required(VERSION 3.29)
project(numatrade)

set(CMAKE_CXX_STANDARD 23)

add_executable(numatrade main.cpp
        src/admin/data/Company.h
        src/admin/data/Company.h
        src/admin/Admin.h
        src/admin/data/enum/Mode.h
        src/admin/data/Instrument.h
        src/sys/Bus.h
        src/sys/NatsBase.h
        src/sys/boot.h
        src/sys/Logger.h
        src/sys/NATSLogger.h
        src/sys/Ipc.h

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