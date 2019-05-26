#include <iostream>
#include <memory>

using namespace std;

struct Test {
  int val;
};



int main(int argc, char *argv[])
{
  unique_ptr<Test> p = make_unique<Test>();
  p->val = 2;

  unique_ptr<Test> p1 = move(p);


  cout << p1->val << endl;
  
  return 0;
}
