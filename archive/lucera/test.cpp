#include <iostream>
#include <mutex>
#include <memory>

using namespace std;

struct Test{
  Test(int i) {
    cout << "constructor" << endl;
  }

  void execute() {
    cout << "do stuff" << endl;
  }

  ~Test() {
    cout << "destructor" << endl;
  }

};

int main(int argc, char *argv[])
{
  shared_ptr<Test> sp1;
  (*sp1).execute();

  return 0;
}
