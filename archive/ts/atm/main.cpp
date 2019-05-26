#include <unordered_map>
#include <memory>

using namespace std;

enum class AccountType : int {
  checking = 1,
  saving = 2
};

template < class ConcreteAccount >
struct Account {


  AccountType type() {
    return ConcreteAccount::type;
  }

  uint32_t deposit(uint32_t amount) {
    this->balance_ += amount;
    return this->balance_;
  }


  uint32_t withdraw(uint32_t amount) {
    check_balance(amount);
    balance_ -= amount;
    return this->balance_;
  }


  uint32_t balance() const {
    return balance_;
  }


  uint32_t transfer_to(Account & rhs, uint32_t amount) {
    this->withdraw(amount);
    rhs.deposit(amount);
  }

protected:
  uint32_t balance_ = 0;
private:
  void check_balance(uint32_t amount) {
    if (amount > balance_) {
      throw std::runtime_error("overdraft");
    }
  }
};



class CheckingAccount: public Account<CheckingAccount> {

  static const AccountType type = AccountType::checking;

};


class SavingAccount: public Account<SavingAccount> {

  static const AccountType type = AccountType::saving;

};


struct User {
  int id;
  string name;


  User(int id, string name)
    :id(id),
     name(name)
  {}

  void add_account(unique_ptr<Account<typename T>> account) {
    if (!this->accounts_.count(account->type())) {
       this->accounts_[account->type()] = account;
    }
  }


  Account * get_account(AccountType & type) {
    this->accounts_[type].get();
  }

private:
  unordered_map<AccountType, unique_ptr<Account<typename T>>> accounts_;
};


using DB = unordered_map<int, User>;

struct ATM {

  ATM(DB & db)
    :curr_user_(nullptr)
    ,logged_(false)
    ,db_(db)

  void login(int user_id) {
    if (db_.count(user_id)) curr_user_ = &db_[user_id];
    logged_ = curr_user_ != nullptr;
  }


  uint32_t withdraw(uint32_t amount) {
    if (this->logged_) {
      return get_account()->withdraw(amount);
    }
    return 0;
  }

  uint32_t deposit(uint32_t amount) {
    if (this->logged_) {
      return get_account()->deposit(amount);
    }
    return 0;
  }


private:
     Account * get_account() {
       return curr_user_->get_account(AccountType::checking).get();
     }

     User * curr_user_;
     bool logged_;
     DB & db_;
};


int main(int argc, char *argv[])
{
  DB user_db;

  User user1(1, "user1"), user2(2, "user2");
  user_db.emplace(user1.id, user1);
  user_db.emplace(user2.id, user2);

  ATM atm(user_db);


  assert(0 == atm.withdraw(1000));

  atm.login(user1.id);

  atm.deposit(100);


  return 0;
}



