#include <iostream>
#include <sstream>

using namespace std;


int main(int argc, char *argv[])
{
  string s;
  while(getline(cin, s)) {
    stringstream ss(s);
    int a;
    ss >> a;
    cout << a << endl;
  } ;
  
  return 0;
}
