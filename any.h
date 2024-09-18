// Copyright (c) 2024 Irina Selyakh
//
// Данное программное обеспечение распространяется на условиях лицензии MIT.
// Подробности смотрите в файле LICENSE

#ifndef MY_ANY_H
#define MY_ANY_H

#include <stdexcept>
#include <memory>
#include <utility>

class BadAnyCast : public std::bad_cast { // обработка ошибок при неправильном приведении типа через функцию AnyCast.
 public:
  [[nodiscard]] const char* what() const noexcept override {
    return "BadAnyCast error";
  }
};

struct IHolder { // абстрактный базовый класс
  virtual std::unique_ptr<IHolder> Clone() const = 0; // ф-ция для клонирования объектов

  virtual ~IHolder() = default;
};

template <class T>
struct AnyHolder : public IHolder { // наследник IHolder, хранит объект типа T и предоставляет возможность клонирования этого объекта
  T value;

  explicit AnyHolder(T p_value) : value(std::move(p_value)) {
  }

  std::unique_ptr<IHolder> Clone() const override {
    return std::make_unique<AnyHolder<T>>(value);
  }
};

class Any { // может хранить объекты любого типа
  std::unique_ptr<IHolder> holder_;

 public:
  Any() : holder_(nullptr) {
  }

  template <class T>
  Any(T p_value) : holder_(std::make_unique<AnyHolder<T>>(std::move(p_value))) {  // NOLINT
  }

  Any(const Any& other) : holder_(other.holder_->Clone()) {
  }

  Any& operator=(const Any& other) {
    if (this != &other) {
      holder_ = other.holder_ ? other.holder_->Clone() : nullptr;
    }
    return *this;
  }

  Any& operator=(Any&& other) noexcept {
    if (this != &other) {
      holder_ = std::move(other.holder_);
      other.holder_ = nullptr;
    }
    return *this;
  }

  void Swap(Any& other) {
    std::swap(holder_, other.holder_);
  }

  void Reset() { // обнуляет объект, делая его пустым
    holder_ = nullptr;
  } 

  bool HasValue() const { // true, если объект не пуст, false иначе
    return holder_ != nullptr;
  }

  template <class T>
  friend T AnyCast(const Any& any); // безопасное извлечение значения из объектов Any
};

template <class T>
T AnyCast(const Any& any) {
  if (auto p = dynamic_cast<AnyHolder<T>*>(any.holder_.get())) {
    return p->value;
  }
  throw BadAnyCast();
}
#endif