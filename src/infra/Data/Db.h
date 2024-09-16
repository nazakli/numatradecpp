#ifndef DB_H
#define DB_H
#include <sqlite3.h>
#include <iostream>
#include <string>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>
#include <unordered_map>
#include <fstream>

class Db {
public:
    // Singleton örneğini alma
    static Db& getInstance() {
        static Db instance;
        return instance;
    }

    // Veritabanına bağlanma, uygulama adı ile aynı isimde veritabanı oluştur
    bool connect() {
        std::string dbName = "trade_engine.db"; // Uygulama adıyla aynı isimde veritabanı
        int rc = sqlite3_open(dbName.c_str(), &db);
        if (rc) {
            logError("Can't open database: " + std::string(sqlite3_errmsg(db)));
            return false;
        }
        logInfo("Opened database successfully: " + dbName);

        // Worker thread başlat
        workerThread = std::thread(&Db::processQueue, this);
        return true;
    }

    // Veritabanı bağlantısını kapatma
    void close() {
        if (db) {
            // Worker thread durdurma
            {
                std::lock_guard<std::mutex> lock(queueMutex);
                stopWorker = true;
            }
            queueCondition.notify_all();
            if (workerThread.joinable()) {
                workerThread.join();
            }

            sqlite3_close(db);
            logInfo("Database closed.");
        }
    }

    // SQL sorgusunu kuyruğa ekle (Non-blocking)
    std::future<bool> executeSQL(const std::string& sql) {
        // logInfo("Queuing SQL command: " + sql);
        std::promise<bool> promise;
        std::future<bool> future = promise.get_future();

        {
            std::lock_guard<std::mutex> lock(queueMutex);
            queryQueue.push({sql, std::move(promise)});
        }

        // Kuyruktaki yeni sorgu için işaret gönder
        queueCondition.notify_one();
        return future;
    }

    // SELECT sorgusu yapma ve sonuçları std::unordered_map ile döndürme
    std::vector<std::unordered_map<std::string, std::string>> querySQL(const std::string& sql) {
        // logInfo("Executing SQL query: " + sql);
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
                const char* columnText = reinterpret_cast<const char*>(sqlite3_column_text(stmt, i));
                row[columnName] = columnText ? columnText : "";
            }
            results.push_back(row);
        }

        sqlite3_finalize(stmt);
        // logInfo("Query executed successfully.");
        return results;
    }

private:
    sqlite3* db = nullptr;
    std::queue<std::pair<std::string, std::promise<bool>>> queryQueue;
    std::thread workerThread;
    std::mutex queueMutex;
    std::condition_variable queueCondition;
    bool stopWorker = false;

    // Kuyruktaki SQL sorgularını işleyen worker thread fonksiyonu
    void processQueue() {
        while (true) {
            std::pair<std::string, std::promise<bool>> queryTask;

            // Kuyruktan iş almak için bekle
            {
                std::unique_lock<std::mutex> lock(queueMutex);
                queueCondition.wait(lock, [this] {
                    return !queryQueue.empty() || stopWorker;
                });

                if (stopWorker && queryQueue.empty()) {
                    break;
                }

                queryTask = std::move(queryQueue.front());
                queryQueue.pop();
            }

            // SQL sorgusunu işleme
            char* errorMessage = nullptr;
            // logInfo("Executing SQL command: " + queryTask.first);
            int rc = sqlite3_exec(db, queryTask.first.c_str(), nullptr, nullptr, &errorMessage);

            if (rc != SQLITE_OK) {
                logError("SQL error: " + std::string(errorMessage));
                sqlite3_free(errorMessage);
                queryTask.second.set_value(false);  // Hata varsa false döner
            } else {
                //logInfo("SQL command executed successfully.");
                queryTask.second.set_value(true);  // Başarılıysa true döner
            }
        }
    }

    // Loglama fonksiyonları
    void logInfo(const std::string& message) {
        std::ofstream logFile("db_log.txt", std::ios_base::app);
        logFile << "[INFO] " << message << std::endl;
        std::cout << "[INFO] " << message << std::endl;
    }

    void logError(const std::string& message) {
        std::ofstream logFile("db_log.txt", std::ios_base::app);
        logFile << "[ERROR] " << message << std::endl;
        std::cerr << "[ERROR] " << message << std::endl;
    }

    // Constructor private, böylece yalnızca Singleton pattern ile erişim sağlanır
    Db() = default;

    // Kopyalama yapılandırıcısı ve kopyalama operatörünü engelle
    Db(const Db&) = delete;
    void operator=(const Db&) = delete;

    // Destructor
    ~Db() {
        close();
    }
};
#endif // DB_H