#include <stdio.h>
#include <stdlib.h>
#include <iostream>

using namespace std;
namespace first
{
  int y = 8;
}

namespace second
{
  double y = 2.1715;
}

class Test{
  public:
    int upper;
    int lower;
    Test(int i)
      : lower(i), upper(lower + 1) {};
};


int i;

void increment(int i) {
  i++;
}


template <typename T>
class Foo {
   T t_;
  public:
    Foo() {}
    Foo (T t): t_(t) {}
};


class D :public Foo<std::string> {};

int main(void)
{

  D d;
  return 0;

}
