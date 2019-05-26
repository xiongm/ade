#include <bits/stdc++.h>

using namespace std;


int main(int argc, char *argv[])
{
  string s = "12";
  string rem = s.substr(2);
  cout << rem << endl;
  if (rem.empty()) {
    cout << "empty" << endl;
  }
  return 0;
}
