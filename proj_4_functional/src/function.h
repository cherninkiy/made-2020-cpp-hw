#pragma once

#include <type_traits>
#include <typeinfo>
#include <utility>

template <typename R, typename... Args>
class function {};

template <typename R, typename... Args>
class function<R(Args...)> {
 public:
  using result_type = R;

  template <typename F>
  struct typeid_ref {
    static std::type_info& value() {
      return const_cast<std::type_info&>(typeid(F));
    }
  };

  template <typename T, typename V>
  static T implicit_cast(V x) {
    return (T*)(void*)x;
  }

  ~function() { target_ = nullptr; }

  function()
      : target_(nullptr), typeid_callback_([]() -> std::type_info& {
          return typeid_ref<void>::value();
        }) {}

  function(std::nullptr_t)
      : target_(nullptr), typeid_callback_([]() -> std::type_info& {
          return typeid_ref<void>::value();
        }) {}

  function(const function& other)
      : target_(other.target_), typeid_callback_(other.typeid_callback_) {}

  function(function&& other)
      : target_(nullptr), typeid_callback_([]() -> std::type_info& {
          return typeid_ref<void>::value();
        }) {
    other.swap(*this);
  }

  template <typename F>
  function(F f)
      : target_(std::move(f)), typeid_callback_([]() -> std::type_info& {
          return typeid_ref<F>::value();
        }) {}

  function& operator=(std::nullptr_t) {
    target_ = nullptr;
    typeid_callback_ = []() -> std::type_info& {
      return typeid_ref<void>::value();
    };
    return *this;
  }

  function& operator=(const function& other) {
    if (this == &other) {
      return *this;
    }
    function(other).swap(*this);
    return *this;
  }

  function& operator=(function&& other) {
    if (this == &other) {
      return *this;
    }
    function().swap(*this);
    other.swap(*this);
    return *this;
  }

  template <class F>
  function& operator=(F&& f) {
    function(std::forward(f)).swap(*this);
    return *this;
  }

  template <class F>
  function& operator=(std::reference_wrapper<F>& f) {
    function(f).swap(*this);
    return *this;
  }

  explicit operator bool() const noexcept { return target_ != nullptr; }

  R operator()(Args... args) { return target_(std::forward<Args>(args)...); }

  void swap(function& other) {
    std::swap(target_, other.target_);
    std::swap(typeid_callback_, other.typeid_callback_);
  }

  const std::type_info& target_type() const {
    return target_ != nullptr ? typeid_callback_() : typeid(void);
  }

  template <class T>
  T* target() {
    return typeid_callback_() == typeid(T*) ? (T*)(void*)target_ : nullptr;
  }

  template <class T>
  const T* target() const {
    return target<T>();
  }

 private:
  R (*target_)(Args...);
  std::type_info& (*typeid_callback_)();
};
