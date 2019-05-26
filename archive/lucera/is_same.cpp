#include <type_traits>
#include <iostream>


template<class T, T v>
struct integral_constant {
  static constexpr T value = v;
  using value_type = T;
};


using true_type = integral_constant<bool, true>;

template<class T, class U>
struct is_same;

template<class T, class U>
struct is_same : std::false_type {};

template<class T>
struct is_same<T, T> : std::true_type {};

static_assert(true_type::value==true, "not good");

static_assert(is_same<int, int>::value == true, "not good");



int main(int argc, char *argv[])
{
  std::cout << is_same<int, int>::value << std::endl;
  return 0;
}
