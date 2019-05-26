#include <bits/stdc++.h>

using namespace std;

int segment_length(const vector<pair<int, int>> &segments) {
  map<int,int> hash;
  for (auto &v : segments) {
    hash[v.first]++;
    hash[v.second]--;
  }

  auto start = 0;
  auto res = 0, sum = 0;
  for (auto p = begin(hash);p != end(hash);) {
    start = p->first;
    sum += p->second;
    while (sum) {
      p++;
      sum += p->second;
    }
    res += p->first - start;
    p++;
  }

  return res;
}



int main(int argc, char *argv[])
{
  assert (8 == segment_length( {{1, 3}, {1, 4}, {2, 4}, {7, 9}, {12, 13}, {13, 15}} ));
  assert (3 == segment_length({{1, 3}, {5,6}}));
  assert (2 == segment_length({{1, 3}, {2, 3}}));


  return 0;
}
