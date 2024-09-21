//
// Created by Numan on 20.09.2024.
//

#ifndef LOG_H
#define LOG_H
#include <filesystem>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/daily_file_sink.h>
#include <iostream>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/async.h>

class Log {
public:
  // Singleton instance
  static Log &getInstance() {
    static Log instance;
    return instance;
  }

  void logInfo(const std::string &message) const {
    logger_->info(message);
  }

  void logWarn(const std::string &message) const {
    logger_->warn(message);
  }

  void logError(const std::string &message) const {
    logger_->error(message);
  }

private:
  std::shared_ptr<spdlog::logger> logger_;

  Log() {
    if (const std::string log_dir = "log"; !std::filesystem::exists(log_dir)) {
      std::filesystem::create_directory(log_dir);
    }

    spdlog::init_thread_pool(8192, 1);
    const auto file_sink = std::make_shared<spdlog::sinks::daily_file_sink_mt>("log/daily_log", 0, 0);
    const auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();

    std::vector<spdlog::sink_ptr> sinks{console_sink, file_sink};
    logger_ = std::make_shared<spdlog::async_logger>("multi_sink", sinks.begin(), sinks.end(), spdlog::thread_pool(), spdlog::async_overflow_policy::block);

    file_sink->set_pattern("[%Y-%m-%d %H:%M:%S.%f] [Thread %t] [%l] %v");
    console_sink->set_pattern("[%Y-%m-%d %H:%M:%S.%f] [Thread %t] [%l] %v");

    spdlog::flush_every(std::chrono::seconds(3));

    spdlog::register_logger(logger_);
  }


  Log(const Log &) = delete;

  Log &operator=(const Log &) = delete;
};

#endif //LOG_H
