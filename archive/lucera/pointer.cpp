#include <iostream>
#include <mutex>

using namespace std;

template<typename T>
struct ControlBlock {
  int ref_count = 0;

  void incre() {
    lock_guard<mutex> l(m_);
    ref_count++;
  }

  int decre() {
    lock_guard<mutex> l(m_);
    return --ref_count;
  }
private:
  mutex m_;
};

template <typename T>
class Simple_SP {
public:

  Simple_SP()
  {
    control_block_ = new ControlBlock<T>();
    control_block_->incre();
  }

  Simple_SP(T *t) {
    control_block_ = new ControlBlock<T>();
    p_ = t;
    control_block_->incre();
  }

  Simple_SP(const Simple_SP<T> & rhs)
    :control_block_(rhs.control_block_){
    control_block_->incre();
  }

  Simple_SP<T> &operator=(const Simple_SP<T> & rhs) {
    if (this != &rhs) {
      this->release_();
      control_block_ = rhs.control_block_;
      p_ = rhs.p_;
      control_block_->incre();
    }
    return *this;
  }


  T & operator*() {
    return *p_;
  }

  T * operator->() {
    return p_;
  }


  ~Simple_SP() {
    this->release_();
  }


private:

  void release_() {
    if (control_block_->decre() == 0) {
      delete p_;
      delete control_block_;
    }
  }

  T *p_ = nullptr;
  ControlBlock<T> *control_block_ = nullptr;
};



struct Test{
  Test() {
    cout << "constructor" << endl;
  }

  void execute() {
    cout << "do stuff" << endl;
  }

  ~Test() {
    cout << "destructor" << endl;
  }

};
int main(int argc, char *argv[])
{
  Simple_SP<Test> sp (new Test());
  sp->execute();
  Simple_SP<Test> sp1 = sp;


  return 0;
}

