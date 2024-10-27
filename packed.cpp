#include <iostream>


using namespace std;

struct Unpacked {
  char a;
  uint32_t b;
  char c;
};

struct __attribute__((packed)) Packed {
  char a;
  uint32_t b;
  char c;
}; 



std::ostream & operator<<(ostream & os, const Unpacked msg)
{
  os << msg.a << " " << msg.b << " " << msg.c << endl;
  return os;
}

std::ostream & operator<<(ostream & os, const Packed msg)
{
  os << msg.a << " " << msg.b << " " << msg.c << endl;
  return os;
}

int main(int argc, char *argv[])
{
  cout << "Size of unpacked:" <<  sizeof(Unpacked) << endl; // 12, alligned on 4 byte integer 
  cout << "Size of packed:" << sizeof(Packed) << endl; // 6

  Unpacked test {'a', 10, 'b'};
  Packed test_packed {'a', 10, 'b'};


  char * buffer = (char *) &test;
  char * buffer_packed = (char *) &test_packed;

  
  auto unpacked_to_unpacked  = * (reinterpret_cast<Unpacked *> (buffer)); // fine
  cout << unpacked_to_unpacked;

  auto packed_to_unpacked  = * (reinterpret_cast<Unpacked *> (buffer_packed)); // undefined
  cout << packed_to_unpacked;

  auto packed_to_packed  = * (reinterpret_cast<Packed *> (buffer_packed)); // fine
  cout << packed_to_packed;

  auto unpacked_to_packed = * (reinterpret_cast<Packed *> (buffer)); //undefined
  cout << unpacked_to_packed;



  return 0;
}
