/*
 *
 * reverse polish notation
 *
 */

#include <functional>
#include <iostream>
#include <unordered_map>
#include <stack>
#include <vector>

using Operands = std::stack<int>;

using RPNOperator = std::function<void (Operands &)>;


class RPNProcessor
{
public:
  void register_operator(char op, RPNOperator func) {
    operators_[op] = func;
  }

  int eval(const std::vector<std::string> & notations) {
     for (auto & n : notations) {
       if (n.size() == 1 && is_supported(n[0])) {
         operators_[n[0]](this->stack_);
       } else {
         stack_.push(stoi(n));
       }
     }

     return stack_.top();
  }


private:

  bool is_supported(char op) {
    return operators_.count(op);
  }

  std::unordered_map<char, RPNOperator> operators_;
  std::stack<int> stack_;
};


struct AddOperator {

  void operator() (Operands & s) {
    auto y = s.top(); s.pop();
    auto x = s.top(); s.pop();
    x += y;

    s.push(x);
  }
};

struct MultiplyOperator {
  void operator() (Operands & s) {
    auto y = s.top(); s.pop();
    auto x = s.top(); s.pop();
    x *= y;

    s.push(x);
  }
};


int main(int argc, char *argv[])
{

  RPNProcessor processor;
  processor.register_operator('+', AddOperator());
  processor.register_operator('*', MultiplyOperator());

  int res = processor.eval({"2", "1", "+", "3", "*"});

  std::cout << res << std::endl;

  return 0;
}
