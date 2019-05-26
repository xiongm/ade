#include <bits/stdc++.h>

using namespace std;


class foo {
public:
  virtual void f() {
    cout << "Base::f" << endl;
  }

  virtual ~foo() = 0;
};

class bar : public foo {
public:
  virtual void f() {
    cout << "Derived::f" << endl;
  }
};

int main(int argc, char *argv[])
{
  bar b;
  foo *f = &b;
  f->f();
  return 0;
}
