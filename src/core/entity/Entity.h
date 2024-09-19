//
// Created by Numan on 18.09.2024.
//

#ifndef BASE_H
#define BASE_H
#include <unordered_map>
#include <string>
#include <iostream>
#include <glaze/glaze.hpp>

class Entity {
public:
  unsigned long long id;

  // Constructor
  explicit Entity(unsigned long long id = 0) : id(id) {}

  // CRUD İşlemleri
  virtual bool create() = 0;
  virtual std::unordered_map<std::string, std::string> read() = 0;
  virtual bool update() = 0;
  virtual bool deleteEntity() = 0;

  // toJson ve toPrint fonksiyonları
  [[nodiscard]] virtual std::string toJson() const = 0;
  virtual void toPrint() const = 0;

  // Destructor
  virtual ~Entity() = default;
};#endif //BASE_H
