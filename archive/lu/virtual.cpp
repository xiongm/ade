#include <bits/stdc++.h>

using namespace std;

struct A
{
  virtual void execute() {
    cout << "a" << endl;
  }
};


struct B : public A
{
  virtual void execute() override {
    cout << "b" << endl;
  }
};


int main(int argc, char *argv[])
{
  B b;
  A a = b;
  a.execute();
  return 0;
}
