#pragma once

#include <type_traits>
#include <utility>

namespace task {

// Forward deaclarations
template <class T>
class UniquePtr;
template <class T>
class SharedPtr;
template <class T>
class WeakPtr;

namespace util {
template <class T>
struct RefCounter {
  T* ptr;
  long use_count;
  long weak_count;

  static T* GetPtr(RefCounter* counter);
  static long UseCount(RefCounter* counter);
  static long WeakCount(RefCounter* counter);

  static RefCounter* SharedCounter(T* ptr);

  static RefCounter* IncrementShared(RefCounter* counter);
  static RefCounter* DecrementShared(RefCounter* counter);

  static RefCounter* IncrementWeak(RefCounter* counter);
  static RefCounter* DecrementWeak(RefCounter* counter);
};
}  // namespace util

template <class T>
class UniquePtr {
 public:
  typedef T* pointer;
  typedef T element_type;
  typedef std::add_lvalue_reference<element_type> lreference_type;

  UniquePtr() : ptr_(nullptr) {}
  ~UniquePtr() { reset(nullptr); }

  UniquePtr(UniquePtr&) = delete;
  UniquePtr(UniquePtr&& other);

  UniquePtr& operator=(UniquePtr&) = delete;
  UniquePtr& operator=(UniquePtr&& other);

  explicit UniquePtr(T* ptr) : ptr_(ptr) {}

  T* get() const { return ptr_; }
  T* operator->() const { return get(); }
  typename lreference_type::type operator*() const { return *get(); }

  T* release();
  void reset(T* ptr);
  void reset(std::nullptr_t ptr = nullptr);
  void swap(UniquePtr<T>& other);

 private:
  T* ptr_;
};

template <class T>
class SharedPtr {
  friend class WeakPtr<T>;
  using Counter = util::RefCounter<T>;

 public:
  typedef std::remove_extent_t<T> element_type;
  typedef std::add_lvalue_reference<element_type> lreference_type;
  typedef WeakPtr<T> weak_type;

  SharedPtr() : refCounter_(nullptr) {}

  ~SharedPtr() { Counter::DecrementShared(refCounter_); }

  SharedPtr(SharedPtr& other)
      : refCounter_(Counter::IncrementShared(other.refCounter_)) {}
  SharedPtr(const SharedPtr& other)
      : refCounter_(Counter::IncrementShared(other.refCounter_)) {}

  SharedPtr(SharedPtr&& other)
      : refCounter_(Counter::IncrementShared(other.refCounter_)) {}
  SharedPtr(const SharedPtr&& other)
      : refCounter_(Counter::IncrementShared(other.refCounter_)) {}

  SharedPtr(const WeakPtr<T>& weak)
      : refCounter_(Counter::IncrementShared(weak.refCounter_)) {}

  explicit SharedPtr(T* ptr) : refCounter_(Counter::SharedCounter(ptr)) {}

  SharedPtr& operator=(SharedPtr& other);
  SharedPtr& operator=(SharedPtr&& other);

  T* get() const { return Counter::GetPtr(refCounter_); }
  typename lreference_type::type operator*() const { return *get(); }
  T* operator->() const { return get(); }

  long use_count() const { return Counter::UseCount(refCounter_); }

  void reset(T* ptr = nullptr);
  void swap(SharedPtr& other);

 private:
  Counter* refCounter_;
};

template <class T>
class WeakPtr {
  friend class SharedPtr<T>;
  using Counter = util::RefCounter<T>;

 public:
  typedef std::remove_extent_t<T> element_type;

  WeakPtr() : refCounter_(nullptr) {}
  ~WeakPtr() { Counter::DecrementWeak(refCounter_); }

  WeakPtr(WeakPtr& other)
      : refCounter_(Counter::IncrementWeak(other.refCounter_)) {}
  WeakPtr(WeakPtr&& other) : refCounter_(nullptr) {
    std::swap(refCounter_, other.refCounter_);
  }

  WeakPtr(const SharedPtr<T>& shared)
      : refCounter_(Counter::IncrementWeak(shared.refCounter_)) {}

  WeakPtr& operator=(WeakPtr& other);
  WeakPtr& operator=(WeakPtr&& other);
  WeakPtr& operator=(SharedPtr<T>& shared);

  SharedPtr<T> lock() const { return SharedPtr<T>(*this); }

  long use_count() const { return Counter::UseCount(refCounter_); }
  bool expired() const { return (use_count() == 0); }

  void reset();
  void swap(WeakPtr<T>& other);

 private:
  Counter* refCounter_;
};

}  // namespace task

#include "smart_pointers.tpp"
