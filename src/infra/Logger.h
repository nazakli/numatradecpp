#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <spdlog/spdlog.h>
#include <spdlog/sinks/daily_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <memory>
#include <filesystem> // For creating log directory

class Logger {
public:
    // Static method to initialize the logger
    static void initialize() {
        if (!logger) {
            try {
                // Create log directory if it doesn't exist
                std::filesystem::create_directory("log");

                // Create a color multi-threaded console logger
                auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
                // Create a daily file logger
                auto file_sink = std::make_shared<spdlog::sinks::daily_file_sink_mt>("log/daily_log.txt", 0, 0);

                // Initialize the logger
                logger = std::make_shared<spdlog::logger>("multi_sink", spdlog::sinks_init_list{console_sink, file_sink});
                spdlog::register_logger(logger);
                spdlog::set_default_logger(logger);
                spdlog::set_level(spdlog::level::debug); // Set global log level to debug
                spdlog::flush_every(std::chrono::seconds(3)); // Flush every 3 seconds
                spdlog::flush_on(spdlog::level::err);  // Flush all loggers on error level
                spdlog::set_pattern("%Y-%m-%d %H:%M:%S.%f %l %v"); // Log format with microseconds
            }
            catch (const spdlog::spdlog_ex &ex) {
                std::cout << "Log initialization failed: " << ex.what() << std::endl;
            }
        }
    }

    // Static method to log an info message
    static void info(const std::string& message) {
        if (logger) {
            logger->info(message);
        }
    }

    // Static method to log a warning message
    static void warn(const std::string& message) {
        if (logger) {
            logger->warn(message);
        }
    }

    // Static method to log an error message
    static void error(const std::string& message) {
        if (logger) {
            logger->error(message);
        }
    }

private:
    // The spdlog instance
    static std::shared_ptr<spdlog::logger> logger;
};


std::shared_ptr<spdlog::logger> Logger::logger = nullptr;

#endif // LOGGER_HPP