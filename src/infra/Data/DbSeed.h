//
// Created by Numan on 19.09.2024.
//

#ifndef DBSEED_H
#define DBSEED_H

#include <string>

#include "Db.h"

struct DbSeed {
  static void createTables() {
    Db& db = Db::getInstance();
    std::cout << "Creating tables..." << std::endl;
    db.execSQL("CREATE TABLE IF NOT EXISTS company (id INTEGER PRIMARY KEY, code TEXT)");
    db.execSQL("CREATE TABLE IF NOT EXISTS client (id INTEGER PRIMARY KEY, name TEXT, email TEXT)");
    db.execSQL("CREATE TABLE IF NOT EXISTS account (id INTEGER PRIMARY KEY, client_id INTEGER, "
                  "balance REAL, equity REAL, free_margin REAL, margin_level REAL, FOREIGN KEY(client_id) REFERENCES client(id))");
    db.execSQL("CREATE TABLE IF NOT EXISTS instrument (id VARCHAR(21) PRIMARY KEY, name TEXT)");
  }
};


#endif //DBSEED_H
