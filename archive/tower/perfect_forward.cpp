#include <bits/stdc++.h>


using namespace std;

void foo(std::string&& s) {
  cout << "calling rvalue reference" << '\n';
  cout << s << '\n';
  this_thread::sleep_for(chrono::seconds(1));
}

void foo(const std::string & s) {
  cout << "calling lvalue" << '\n';
  cout << s << '\n';
  this_thread::sleep_for(chrono::seconds(1));
}


template<typename T>
auto time_foo(T&& param) {
  auto before = chrono::system_clock::now();
  foo(forward<T>(param));
  auto after = chrono::system_clock::now();

  auto elapsed = chrono::duration_cast<chrono::milliseconds>(after - before);

  return elapsed.count();
}

template<typename Func, typename...Params>
auto time_it(Func&& func, Params &&... params)
{
  auto before = chrono::system_clock::now();
  forward<Func>(decltype(func))(forward<Params>(params)...);
  auto after = chrono::system_clock::now();

  auto elapsed = chrono::duration_cast<chrono::milliseconds>(after - before);

  return elapsed.count();

}

int main(int argc, char *argv[])
{
  auto l = time_foo("hello world");
  std::string && s = std::string("hello world");
  l = time_foo(s);
  cout << l << '\n';

  l = time_it(foo, "hello world");
  
  cout << l << '\n';
  return 0;
}
