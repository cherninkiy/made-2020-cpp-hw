#include <cassert>
#include <functional>
#include <iostream>

#include "function.h"

using std::cout;
using std::endl;

void nothing() {}

template <typename T>
T identity_x(T arg) {
  return arg;
}

int identity_i(int arg) { return arg; }

int add(int a, int b) { return a + b; }

int func_static(int a) { return -a; }

auto lambda_static = [](int a) { return -a; };

int main() {
  auto lambda_local = [](int a) { return -a; };

  {  // Target and result types test

    function<void(void)> fn1, fn2(nothing);

    assert(typeid(decltype(fn1)::result_type) == typeid(void));
    assert(fn1.target_type() == typeid(void));

    assert(typeid(decltype(fn1)::result_type) ==
           typeid(decltype(fn2)::result_type));
    assert(fn1.target_type() != fn2.target_type());

    function<int(int)> fn3(identity_x<int>), fn4(identity_i);

    assert(typeid(decltype(fn3)::result_type) == typeid(int));
    assert(fn3.target_type() == typeid(int (*)(int)));

    assert(typeid(decltype(fn3)::result_type) ==
           typeid(decltype(fn4)::result_type));
    assert(fn3.target_type() == fn4.target_type());

    cout << "PASSED: Target and result types test" << endl;
  }

  {  // Copy and assign initialization
    function<int(int)> fn1(func_static);

    auto fn2(fn1);
    assert(fn1);
    assert(fn2);

    auto fn3 = fn1;
    assert(fn1);
    assert(fn3);

    auto fn4 = std::move(fn1);
    assert(!fn1);
    assert(fn4);

    fn1.swap(fn2);
    assert(fn1);
    assert(!fn2);

    cout << "PASSED: Copy and assignable initialization" << endl;
  }

  {  // Excplicit initialization
    function<int(int)> fn1(func_static), fn2(lambda_static), fn3(lambda_local);

    assert(typeid(decltype(fn1)::result_type) == typeid(int));
    assert(fn1.target_type() == typeid(int (*)(int)));

    assert(typeid(decltype(fn2)::result_type) == typeid(int));
    assert(fn2.target_type() != typeid(int (*)(int)));

    assert(typeid(decltype(fn1)::result_type) ==
           typeid(decltype(fn2)::result_type));
    assert(fn1.target_type() != fn2.target_type());

    cout << "PASSED: Excplicit initialization" << endl;
  }

  {  // Target explicit invocation
    function<int(int)> fn1(identity_x<int>), fn2(identity_i);
    assert(fn1.target<int(int)>()(100) == fn2.target<int(int)>()(100));

    function<int(int, int)> fn3(add);
    assert(fn3.target<int(int, int)>()(2, 2) == 4);

    auto t = fn3.target<void(int, int)>();
    assert(fn3.target<void(int, int)>() == nullptr);

    cout << "PASSED: Target explicit invocation" << endl;
  }

  cout << "PASSED: All tests passed!!!" << endl;
  return 0;
}