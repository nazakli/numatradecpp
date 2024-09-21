//
// Created by Numan on 18.09.2024.
//

#ifndef BASE_H
#define BASE_H
#include <unordered_map>
#include <string>


class Db;

class Entity {
protected:
  std::string tableName;  // Türetilen sınıflar için tablo adı
  Db& db = Db::getInstance();  // Her Entity için tek bir DB instance

public:
   int64_t id;

  // Constructor
  explicit Entity(unsigned long long id = 0, std::string  tableName = "")
      : id(id), tableName(std::move(tableName)) {}

  // CRUD işlemleri virtual olarak tanımlandı
  virtual bool create(const std::unordered_map<std::string, std::string>& data) = 0;
  virtual std::unordered_map<std::string, std::string> read(unsigned long long id) = 0;
  virtual bool update(const std::unordered_map<std::string, std::string>& data) = 0;
  virtual bool deleteEntity() = 0;

  // toJson ve toPrint fonksiyonları virtual olarak tanımlandı
  [[nodiscard]] virtual std::string toJson() const = 0;
  virtual void toPrint() const = 0;

  // Destructor
  virtual ~Entity() = default;
};

#endif //BASE_H
