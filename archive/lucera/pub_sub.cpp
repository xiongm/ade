#include <queue>
#include <condition_variable>
#include <mutex>
#include <assert.h>

using namespace std;

queue<int> q;
condition_variable cv;
mutex m;

class Producer
{
public:
  void write(int payload) {
    lock_guard<mutex> lg(m);
    q.push(payload);
    cv.notify_one();
  }
};


class Consumer
{
public:
  int read() {
    unique_lock<mutex> l(m);
    cv.wait(l, [](){return !q.empty();});
    auto res = q.front(); q.pop();
    l.unlock();
    return res;
  }
};



int main(int argc, char *argv[])
{
  Producer p;
  p.write(1);

  Consumer c;

  assert(1 == c.read());
  assert (q.empty());

  return 0;
}
