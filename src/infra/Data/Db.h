#ifndef DB_H
#define DB_H
#include <fstream>
#include <future>
#include <iostream>
#include <mutex>
#include <queue>
#include <sqlite3.h>
#include <string>
#include <thread>
#include <unordered_map>
#include "../Log/Log.h"
class Db {
public:
    // Singleton örneğini alma
    static Db& getInstance() {
        static Db instance;
        return instance;
    }

    // Veritabanına bağlanma, uygulama adı ile aynı isimde veritabanı oluştur
    bool connect() {
        const std::string dbName = "trade_engine.db"; // Uygulama adıyla aynı isimde veritabanı
        if (const int rc = sqlite3_open(dbName.c_str(), &db)) {
            logError("Can't open database: " + std::string(sqlite3_errmsg(db)));
            return false;
        }
        logInfo("Opened database successfully: " + dbName);

        // Worker thread'leri başlat
        execWorkerThread = std::thread(&Db::processExecQueue, this);
        queryWorkerThread = std::thread(&Db::processQueryQueue, this);
        return true;
    }

    // Veritabanı bağlantısını kapatma
    void close() {
        if (db) {
            // Exec worker thread durdurma
            {
                std::lock_guard<std::mutex> lock(execQueueMutex);
                stopExecWorker = true;
            }
            execQueueCondition.notify_all();
            if (execWorkerThread.joinable()) {
                execWorkerThread.join();
            }

            // Query worker thread durdurma
            {
                std::lock_guard<std::mutex> lock(queryQueueMutex);
                stopQueryWorker = true;
            }
            queryQueueCondition.notify_all();
            if (queryWorkerThread.joinable()) {
                queryWorkerThread.join();
            }

            sqlite3_close(db);
            logInfo("Database closed.");
        }
    }

    // Senkron SQL sorgusu (Non-blocking için)
    bool execSQL(const std::string& sql) {
        logInfo("Executing SQL command synchronously: " + sql);
        char* errorMessage = nullptr;

        if (const int rc = sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &errorMessage); rc != SQLITE_OK) {
            logError("SQL error: " + std::string(errorMessage));
            sqlite3_free(errorMessage);
            return false;
        }

        logInfo("SQL command executed successfully.");
        return true;
    }

    // SQL sorgusunu kuyruğa ekle (exec için asenkron)
    std::future<bool> execSQLAsync(const std::string& sql) {
        std::promise<bool> promise;
        std::future<bool> future = promise.get_future();

        {
            std::lock_guard<std::mutex> lock(execQueueMutex);
            execQueue.emplace(sql, std::move(promise));
        }

        // Kuyruktaki yeni sorgu için işaret gönder
        execQueueCondition.notify_one();
        return future;
    }

    // SQL sorgusunu kuyruğa ekle (query için asenkron)
    std::future<std::vector<std::unordered_map<std::string, std::string>>> querySQLAsync(const std::string& sql) {
        std::promise<std::vector<std::unordered_map<std::string, std::string>>> promise;
        std::future<std::vector<std::unordered_map<std::string, std::string>>> future = promise.get_future();

        {
            std::lock_guard<std::mutex> lock(queryQueueMutex);
            queryQueue.emplace(sql, std::move(promise));
        }

        // Kuyruktaki yeni sorgu için işaret gönder
        queryQueueCondition.notify_one();
        return future;
    }

    // SELECT sorgusu yapma ve sonuçları std::unordered_map ile döndürme (senkron)
    [[nodiscard]] std::vector<std::unordered_map<std::string, std::string>> querySQL(const std::string& sql) const {
        logInfo("Executing SQL query: " + sql);
        std::vector<std::unordered_map<std::string, std::string>> results;
        sqlite3_stmt* stmt;

        int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
        if (rc != SQLITE_OK) {
            logError("Failed to prepare statement: " + std::string(sqlite3_errmsg(db)));
            return results;
        }

        int columns = sqlite3_column_count(stmt);

        while (sqlite3_step(stmt) == SQLITE_ROW) {
            std::unordered_map<std::string, std::string> row;
            for (int i = 0; i < columns; ++i) {
                const char* columnName = sqlite3_column_name(stmt, i);
                const auto columnText = reinterpret_cast<const char*>(sqlite3_column_text(stmt, i));
                row[columnName] = columnText ? columnText : "";
            }
            results.push_back(row);
        }

        sqlite3_finalize(stmt);
        return results;
    }

private:
    sqlite3* db = nullptr;

    // Exec ve Query için ayrı kuyruklar ve thread'ler
    std::queue<std::pair<std::string, std::promise<bool>>> execQueue;
    std::queue<std::pair<std::string, std::promise<std::vector<std::unordered_map<std::string, std::string>>>>> queryQueue;

    std::thread execWorkerThread;
    std::thread queryWorkerThread;

    std::mutex execQueueMutex;
    std::mutex queryQueueMutex;

    std::condition_variable execQueueCondition;
    std::condition_variable queryQueueCondition;

    bool stopExecWorker = false;
    bool stopQueryWorker = false;

    // Exec kuyruğundaki SQL sorgularını işleyen worker thread fonksiyonu
    void processExecQueue() {
        while (true) {
            std::pair<std::string, std::promise<bool>> queryTask;

            // Kuyruktan iş almak için bekle
            {
                std::unique_lock<std::mutex> lock(execQueueMutex);
                execQueueCondition.wait(lock, [this] {
                    return !execQueue.empty() || stopExecWorker;
                });

                if (stopExecWorker && execQueue.empty()) {
                    break;
                }

                queryTask = std::move(execQueue.front());
                execQueue.pop();
            }

            // SQL sorgusunu işleme
            char* errorMessage = nullptr;
            logInfo("Executing SQL exec command: " + queryTask.first);
            int rc = sqlite3_exec(db, queryTask.first.c_str(), nullptr, nullptr, &errorMessage);

            if (rc != SQLITE_OK) {
                logError("SQL error: " + std::string(errorMessage));
                sqlite3_free(errorMessage);
                queryTask.second.set_value(false);  // Hata varsa false döner
            } else {
                logInfo("SQL exec command executed successfully.");
                queryTask.second.set_value(true);  // Başarılıysa true döner
            }
        }
    }

    // Query kuyruğundaki SQL sorgularını işleyen worker thread fonksiyonu
    void processQueryQueue() {
        while (true) {
            std::pair<std::string, std::promise<std::vector<std::unordered_map<std::string, std::string>>>> queryTask;

            // Kuyruktan iş almak için bekle
            {
                std::unique_lock<std::mutex> lock(queryQueueMutex);
                queryQueueCondition.wait(lock, [this] {
                    return !queryQueue.empty() || stopQueryWorker;
                });

                if (stopQueryWorker && queryQueue.empty()) {
                    break;
                }

                queryTask = std::move(queryQueue.front());
                queryQueue.pop();
            }

            // SQL SELECT sorgusunu işleme
            logInfo("Executing SQL query asynchronously: " + queryTask.first);
            std::vector<std::unordered_map<std::string, std::string>> results;
            sqlite3_stmt* stmt;

            int rc = sqlite3_prepare_v2(db, queryTask.first.c_str(), -1, &stmt, nullptr);
            if (rc != SQLITE_OK) {
                logError("Failed to prepare statement: " + std::string(sqlite3_errmsg(db)));
                queryTask.second.set_value(results);  // Hata durumunda boş sonuç döner
                continue;
            }

            int columns = sqlite3_column_count(stmt);

            while (sqlite3_step(stmt) == SQLITE_ROW) {
                std::unordered_map<std::string, std::string> row;
                for (int i = 0; i < columns; ++i) {
                    const char* columnName = sqlite3_column_name(stmt, i);
                    const auto columnText = reinterpret_cast<const char*>(sqlite3_column_text(stmt, i));
                    row[columnName] = columnText ? columnText : "";
                }
                results.push_back(row);
            }

            sqlite3_finalize(stmt);
            queryTask.second.set_value(results);  // Sonuçları promise ile döndür
        }
    }

    static void logInfo(const std::string& message) {
        Log::getInstance().logInfo(message);
    }

    static void logError(const std::string& message) {
        Log::getInstance().logError(message);
    }


    Db() {
        if(!connect()) {
            throw std::runtime_error("Failed to open database.");
        }
    };

    Db(const Db&) = delete;
    void operator=(const Db&) = delete;

    // Destructor
    ~Db() {
        close();
    }
};

#endif // DB_H
