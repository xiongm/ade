
#include <iostream>
#include <stdio.h>

using namespace std;

template<class Derived>
class LineReader {
public:


    void run() {
      string line;
      while(getline(std::cin, line)) {
        derived()->handle(line);
      }
    }


private:
    Derived * derived() {
      return static_cast<Derived *>(this);
    }


};

template <typename TLineReader>
class MessageHandler : public typename TLineReader<MessageHandler>
{
  public:
    void handle(std::string & line) {
      cout << "hello" << endl;
    }
};

int main(int argc, char *argv[])
{
  MessageHandler<LineReader> msg_handler;

  msg_handler.run();
  
  return 0;
}
