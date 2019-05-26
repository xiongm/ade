#include <bits/stdc++.h>

using namespace std;

int main(int argc, char *argv[])
{
  map<int, int > s = {{1, 0},  {2, 0}, {3, 0}, {4, 0}};


  for (auto p = begin(s); p != end(s);) {
    if (p->first & 1) {
      p = s.erase(p);
    } else ++p;
  }

  //s.erase(remove_if(begin(s), end(s), [](pair<int, int> x) { return x.first & 1; }), s.end());


  for_each(begin(s), end(s), [](pair<int, int> x) { cout << x.first;});

  cout << '\n';
  
  return 0;
}
