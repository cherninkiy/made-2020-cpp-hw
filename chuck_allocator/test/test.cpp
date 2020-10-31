#include "chunk_allocator.h"
#include <string>
#include <list>
#include <vector>
#include <memory>
#include <iostream>
#include <typeinfo>

using namespace std;

void FailWithMsg(const std::string& msg, int line) {
  std::cerr << "Test failed!\n";
  std::cerr << "[Line " << line << "] " << msg << std::endl;
  std::exit(EXIT_FAILURE);
}

#define ASSERT_TRUE(cond)                              \
  if (!(cond)) {                                       \
    FailWithMsg("Assertion failed: " #cond, __LINE__); \
  };

#define ASSERT_EXCEPTION_MSG(cond, ex, msg) \
    {bool ok = false;                       \
    try {(cond);} catch (const ex&) {ok = true;} catch (...) {} \
    if (!ok) FailWithMsg(msg, __LINE__);}

#define PRINT(ptr, count)              \
  for (size_t i = 0; i < count; ++i) { \
    cout << ptr[i] << ' ';             \
  }                                    \
  cout << endl;

#define FILL(ptr, value, count)        \
  for (size_t i = 0; i < count; ++i) { \
    ptr[i] = value;                    \
  }

#define RANGE(ptr, start, count)       \
  for (size_t i = 0; i < count; ++i) { \
    ptr[i] = start + i;                \
  }

#define REPEAT(count) for (size_t _iter = 0; _iter < count; ++_iter)

class Foo {
 public:
  int a;
  double d;

  Foo(int a) : Foo(a, 0.0) {}

  Foo(int a, double d) : a(a), d(d) {}
};

template <typename T, typename Alloc>
class LinkedList {
 public:
  struct Node {
    T value;
    Node* prev;
  };

  typedef typename Alloc::template rebind<Node>::other NodeAllocator;

 private:
  NodeAllocator alloc;
  Node* tail = nullptr;

 public:
  LinkedList() {}

  ~LinkedList() {
    for (; tail != nullptr;) {
      Node* nodePtr = tail;
      tail = tail->prev;
      alloc.deallocate(nodePtr, 1);
    }
  }

  void Add(T value) {
    auto nodePtr = alloc.allocate(1);
    nodePtr->value = value;
    nodePtr->prev = tail;
    tail = nodePtr;
  }

  void Remove(T value) {
    if (tail == nullptr) {
      return;
    }

    if (tail->value == value) {
      Node* nodePtr = tail;
      tail = tail->prev;
      alloc.deallocate(nodePtr, 1);
      return;
    }

    Node* nextPtr = tail;
    for (Node *nodePtr = tail->prev; nodePtr->prev != nullptr;
         nodePtr = nodePtr->prev, nextPtr = nextPtr->prev) {
      if (nodePtr->value == value) {
        nextPtr->prev = nodePtr->prev;
        alloc.deallocate(nodePtr, 1);
        return;
      }
    }
  }

  void Resize(size_t newsize, T value) {
    auto nodePtr = alloc.allocate(newsize);
    for (size_t i = 0; i < newsize; ++i) {
      nodePtr->value = value;
      nodePtr->prev = tail;
      tail = nodePtr;
      nodePtr++;
    }
  }
};

int main(int argc, char** argv) {
  const size_t CHUNK_SIZE = (1 << 12);

  // Member types tests
  {
    ASSERT_TRUE(typeid(chunk_allocator<int>::value_type) == typeid(int));
    ASSERT_TRUE(typeid(chunk_allocator<Foo>::value_type) == typeid(Foo));

    ASSERT_TRUE(typeid(chunk_allocator<int>::pointer) == typeid(int*));
    ASSERT_TRUE(typeid(chunk_allocator<Foo>::pointer) == typeid(Foo*));

    ASSERT_TRUE(typeid(chunk_allocator<int>::const_pointer) ==
                typeid(const int*));
    ASSERT_TRUE(typeid(chunk_allocator<Foo>::const_pointer) ==
                typeid(const Foo*));

    ASSERT_TRUE(typeid(chunk_allocator<int>::reference) == typeid(int&));
    ASSERT_TRUE(typeid(chunk_allocator<Foo>::reference) == typeid(Foo&));

    ASSERT_TRUE(typeid(chunk_allocator<int>::const_reference) ==
                typeid(const int&));
    ASSERT_TRUE(typeid(chunk_allocator<Foo>::const_reference) ==
                typeid(const Foo&));

    ASSERT_TRUE(typeid(chunk_allocator<int>::size_type) ==
                typeid(chunk_allocator<Foo>::size_type));

    ASSERT_TRUE(typeid(LinkedList<Foo, chunk_allocator<Foo>>::NodeAllocator) ==
                typeid(chunk_allocator<LinkedList<Foo, chunk_allocator<Foo>>::Node>));
  }

  // Memory reusing test
  {
    chunk_allocator<int> allocator;

    int* tmp = allocator.allocate(8);
    FILL(tmp, -1, 8);

    auto a1 = allocator.allocate(8);
    RANGE(a1, 0, 8);

    auto a2 = allocator.allocate(8);
    RANGE(a2, 100, 8);

    allocator.deallocate(a1, 8);
    for (size_t i = 0; i < 8; ++i) {
      ASSERT_TRUE(a1[i] == i);
    }

    auto a11 = allocator.allocate(8);
    ASSERT_TRUE(a1 == a11);
    for (size_t i = 0; i < 8; ++i) {
      ASSERT_TRUE(a11[i] == i);
    }
  }

  // Memory leakage test
  {
    chunk_allocator<int> allocator;

    int* tmp = allocator.allocate(8);
    FILL(tmp, -1, 8);

    REPEAT(1000) {
      auto a1 = allocator.allocate(8);
      allocator.deallocate(a1, 8);
    }
  }

  // Bad allocation test
  {
    chunk_allocator<uint8_t> allocator;
    ASSERT_EXCEPTION_MSG(allocator.allocate(CHUNK_SIZE + 1), out_of_range, "allocate");
  }

  // Chunks chain test
  {
    chunk_allocator<uint8_t> allocator;

    auto* a0 = allocator.allocate(8);

    // second chunk
    auto a1 = allocator.allocate(CHUNK_SIZE);
    // first chunk
    auto a2 = allocator.allocate(CHUNK_SIZE / 2);
    // third chunk
    auto a3 = allocator.allocate(CHUNK_SIZE / 2);
    // third chunk (tail)
    auto a4 = allocator.allocate(16);
    // first chunk
    auto a5 = allocator.allocate(CHUNK_SIZE / 2 - 8);

    ASSERT_TRUE(allocator.chunk_count() == 3);

    allocator.deallocate(a1, CHUNK_SIZE);
    ASSERT_TRUE(allocator.chunk_count() == 2);

    allocator.deallocate(a0, 8);
    allocator.deallocate(a2, CHUNK_SIZE / 2);
    allocator.deallocate(a4, 16);
    ASSERT_TRUE(allocator.chunk_count() == 1);

    allocator.deallocate(a3, CHUNK_SIZE / 2);
    allocator.deallocate(a5, CHUNK_SIZE / 2 - 8);
    ASSERT_TRUE(allocator.chunk_count() == 0);
  }

  // Construction test
  {
    chunk_allocator<Foo> allocator;

    Foo* fp = allocator.allocate(8);
    Foo* ptr = fp;
    for (int i = 0; i < 8; ++i) {
      allocator.construct(ptr, i);
      ptr += 1;
    }

    ptr = fp;
    for (int i = 0; i < 8; ++i) {
      ASSERT_TRUE(ptr->a == i && ptr->d == 0.0);
      allocator.destroy(ptr);
      ptr += 1;
    }

    ptr = fp;
    for (int i = 0; i < 8; ++i) {
      allocator.construct(ptr, 1, -1.0 * i);
      ptr += 1;
    }

    ptr = fp;
    for (int i = 0; i < 8; ++i) {
      ASSERT_TRUE(ptr->a == 1 && ptr->d == -1.0 * i);
      allocator.destroy(ptr);
      ptr += 1;
    }

    allocator.deallocate(fp, 8);
  }

  // Reference counting
  {
    chunk_allocator<uint8_t> allocator1;
    uint8_t* a1 = allocator1.allocate(CHUNK_SIZE / 2 + 1);

    auto allocator2(allocator1);
    uint8_t* a2 = allocator2.allocate(CHUNK_SIZE / 2 + 1);
    ASSERT_TRUE(allocator2.chunk_count() == 2);

    auto allocator3 = allocator1;
    uint8_t* a3 = allocator3.allocate(CHUNK_SIZE / 2 + 1);
    ASSERT_TRUE(allocator3.chunk_count() == 3);

    auto& allocator4(allocator2);
    uint8_t* a4 = allocator4.allocate(CHUNK_SIZE / 2 + 1);
    ASSERT_TRUE(allocator4.chunk_count() == 4);

    auto& allocator5 = allocator1;
    uint8_t* a5 = allocator5.allocate(CHUNK_SIZE / 2 + 1);
    ASSERT_TRUE(allocator5.chunk_count() == 5);

    auto&& allocator6(allocator2);
    uint8_t* a6 = allocator6.allocate(CHUNK_SIZE / 2 + 1);
    ASSERT_TRUE(allocator6.chunk_count() == 6);

    auto&& allocator7 = allocator2;
    uint8_t* a7 = allocator7.allocate(CHUNK_SIZE / 2 + 1);
    ASSERT_TRUE(allocator7.chunk_count() == 7);

    ASSERT_TRUE(allocator1.reference_count() == 3);
    ASSERT_TRUE(allocator7.reference_count() == 3);

    allocator7.deallocate(a7, CHUNK_SIZE / 2 + 1);
    allocator6.deallocate(a6, CHUNK_SIZE / 2 + 1);
    allocator5.deallocate(a5, CHUNK_SIZE / 2 + 1);
    allocator4.deallocate(a4, CHUNK_SIZE / 2 + 1);

    ASSERT_TRUE(allocator1.chunk_count() == 3);
    ASSERT_TRUE(allocator1.reference_count() == 3);

    auto allocptr = new chunk_allocator<uint8_t>(allocator1);
    ASSERT_TRUE(allocptr->reference_count() == 4);
    ASSERT_TRUE(allocator1.reference_count() == 4);
    delete allocptr;

    ASSERT_TRUE(allocator1.reference_count() == 3);
  }

  // Container constructing
  {
    LinkedList<int, chunk_allocator<int>> lst;
    lst.Add(0);
    lst.Remove(0);
    lst.Resize(8, 1);
  }

  cout << "All tests passed!" << endl;
  return 0;
}