#include <cstdint>
#include <utility>
#include <stdexcept>

template <typename T>
struct chunk_allocator {
 public:
  // default chunk_size is 4 KiB
  static const size_t chunk_size = (1 << 12);

 private:
  class chunk {
    // fragments contain pointers to allocated spaces
    // and within chunk are organized as linked list
    class fragment {
     public:
      // pointer to the begining of the fragment
      uint8_t* ptr;
      // length of the fragment space
      size_t len;
      // pointer to the next allocated fragment of the fragment
      fragment* next;

      fragment(uint8_t* ptr, size_t len, fragment* next)
          : ptr(ptr), len(len), next(next) {}
    };

    // pointer to the begining of the allocated memory
    uint8_t* data_ = nullptr;

    // size of the allocated memory
    // can't be modified after instantiation
    size_t size_ = 0;

    // head fragment
    fragment* head_ = nullptr;

    // previous chunk
    chunk* prev_ = nullptr;

   public:
    chunk(size_t size, chunk* prev) : prev_(prev), size_(size) {
      data_ = new uint8_t[sizeof(uint8_t) * size];
    }
    ~chunk() { delete[] data_; }

    // get pointer to the previous chunk
    chunk* prev() const { return prev_; }

    // set pointer to the previous chunk
    void set_prev(chunk* prev) { prev_ = prev; }

    // split fragment then there is deallocating the middle of it 
    void split(fragment* current, uint8_t* ptr, size_t n) {
      size_t ost_len = current->len - n;
      uint8_t* ost_ptr = current->ptr + (ost_len - (ptr - current->ptr));
      fragment* ost = new fragment(ost_ptr, ost_len, current->next);
      current->len = (ptr - current->ptr);
      current->next = ost;
    }

    // engage new fragment
    uint8_t* engage(size_t n) {
      if (n > size_) {
        throw std::out_of_range("Requested memeory is out of range");
      }

      if (head_ == nullptr) {
        head_ = new fragment(data_, n, nullptr);
        return head_->ptr;
      }

      fragment* current = head_;
      for (fragment* following = current->next;
           following != nullptr && following->ptr != nullptr;
           following = following->next) {
        if (n <= following->ptr - (current->ptr + current->len)) {
          current->next =
              new fragment(current->ptr + current->len, n, following);
          return current->next->ptr;
        }
        current = following;
      }

      if (n <= (data_ + size_) - (current->ptr + current->len)) {
        current->next = new fragment(current->ptr + current->len, n, nullptr);
        return current->next->ptr;
      }

      return nullptr;
    }

    // release fragment space or part of it
    void release(uint8_t* ptr, size_t n) {
      if (head_ == nullptr || head_->ptr == nullptr) {
        return;
      }

      if (head_->ptr == ptr) {
        if (n == head_->len) {
          delete head_;
          head_ = nullptr;
        } else if (n < head_->len) {
          head_->ptr += n;
        }
        return;
      }
      if (head_->ptr < ptr && ptr + n < (head_->ptr + head_->len)) {
        split(head_, ptr, n);
        return;
      }

      fragment* current = head_;
      for (fragment* following = current->next;
           following != nullptr && following->ptr != nullptr;
           following = following->next) {

        if (following->ptr == ptr) {
          if (n == following->len) {
            current->next = following->next;
            delete following;
          } else if (n < following->len) {
            following->ptr += n;
          }
          return;
        }
        if (following->ptr < ptr && ptr + n < (following->ptr + following->len)) {
          split(following, ptr, n);
          return;
        }
        current = following;
      }
    }

    // check if fragment contains allocated address
    bool contains(uint8_t* ptr) {
      return data_ <= ptr && ptr < (data_ + size_);
    }
    
    // check if fragment is empty
    bool empty() { return head_ == nullptr; }
  };

  struct chunk_shares {
    chunk* tail;
    size_t counter;
  };

  chunk_shares* shares_;

 public:
  using value_type = T;
  using pointer = T*;
  using const_pointer = const T*;
  using reference = T&;
  using const_reference = const T&;
  using size_type = std::size_t;

  template <typename U>
  struct rebind { typedef chunk_allocator<U> other; };

  chunk_allocator() {
    shares_ = new chunk_shares();
    shares_->tail = nullptr;
    shares_->counter = 1;
  }

  chunk_allocator(const chunk_allocator& other) : shares_(other.shares_) {
    ++shares_->counter;
  }

  chunk_allocator(const chunk_allocator&& other) = delete;

  chunk_allocator& operator=(const chunk_allocator& other) {
    if (this == &other) {
      return *this;
    }
    return chunk_allocator(*this);
  }

  chunk_allocator&& operator=(const chunk_allocator&& other) = delete;

  ~chunk_allocator() {
    if (--shares_->counter == 0) {
      for (chunk* cur = shares_->tail; 
           shares_->tail != nullptr; cur = shares_->tail) {
        shares_->tail = cur->prev();
        delete cur;
      }
      delete shares_;
    }
  }

  T* allocate(std::size_t n) {
    if (shares_->tail == nullptr) {
      shares_->tail = new chunk(chunk_size, nullptr);
    }
    auto result = shares_->tail->engage(sizeof(T) * n);
    if (result != nullptr) {
      return (T*)(result);
    }

    for (chunk* current = shares_->tail->prev(); 
         current != nullptr; current = current->prev()) {
      result = current->engage(sizeof(T) * n);
      if (result != nullptr) {
        return (T*)(result);
      }
    }

    shares_->tail = new chunk(chunk_size, shares_->tail);
    result = shares_->tail->engage(sizeof(T) * n);
    return (T*)(result);
  }

  void deallocate(T* p, const size_type n) {
    uint8_t* ptr = (uint8_t*)p;
    chunk* previous = shares_->tail;
    for (chunk* current = previous; current != nullptr;
         current = current->prev()) {
      if (current->contains(ptr)) {
        current->release(ptr, sizeof(T) * n);

        if (current->empty()) {
          if (current == shares_->tail) {
            previous = shares_->tail->prev();
            delete shares_->tail;
            shares_->tail = previous;
          } else {
            previous->set_prev(current->prev());
            delete current;
          }
        }

        return;
      }
      previous = current;
    }
  }

  template <typename... Args>
  void construct(T* p, Args&&... args) {
    new ((void*)p) T(std::forward<Args>(args)...);
  }

  void destroy(T* p) { p->~T(); }

  size_t chunk_count() {
    size_t count = 0;
    for (chunk* cur = shares_->tail; 
         cur != nullptr; cur = cur->prev()) {
      count += 1;
    }
    return count;
  }

  size_t reference_count() {
    return shares_->counter;
  }
};
