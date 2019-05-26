#include <iostream>
#include <bitset>

using namespace std;

int main(int argc, char *argv[])
{
  int a = 0b0110110;

  cout << bitset<8>(a) << endl;

  cout << "setting 4th bit from right to left" << endl;

  // 0110110 => 0111110
  a |= 1 << 3;
  cout << bitset<8>(a) << endl;

  cout << "clearing 2nd bit from right to left" << endl;

  // 0111110 => 0111100
  a &= ~(1 << 1);
  cout << bitset<8>(a) << endl;

  cout << "toggle 5th bit from right to left" << endl;

  // 0111100 => 0101100
  a ^= 1 << 4;
  cout << bitset<8>(a) << endl;

  cout << "toggle 5th bit again from right to left" << endl;

  // 0101100 => 0111100
  a ^= 1 << 4;
  cout << bitset<8>(a) << endl;


  cout << "checking 8th bit" << endl;
  bool bit = (a >> 7) & 1;

  cout << boolalpha << bit << endl;


  a = 0b0110110;
  int b = 0b1000000;
  cout << "copying 3rd bit from a(" << bitset<8>(a) << ") to b(" << bitset<8>(b) << ")" << endl;

  // calculate write bit (by reading bit and then setting bit)
  int write_bit = ((a >> 2) & 1) << 2;

  // clearing b's 3rd bit using above operations
  b &= ~(1 << 2);

  // now add the write_bit to b
  b |= write_bit;

  cout << bitset<8>(b) << endl;







  return 0;
}
