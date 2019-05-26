#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <assert.h>
#include <iostream>
#include <memory>

using namespace std;

int get_next_account_number()
{
  static int account_number = 1;
  return account_number++;
}

int get_next_user_id()
{
  static int user_id = 1;
  return user_id++;
}


struct User {
  User(string name)
    :id(get_next_user_id())
    ,name(name)
  {}
  int id;
  string name;
  unordered_set<int> accounts;
};

struct Account {

  enum Kind {CHECKING, SAVING};


  int user_id;
  int account_number;
  Kind kind;
  int balance;

  Account(User &user, Kind kind, int balance)
    :user_id(user.id)
    ,account_number(get_next_account_number())
    ,kind(kind)
    ,balance(balance)
  {
    user.accounts.insert(account_number);
  }

};


struct BankAccounts {

  int add_account(shared_ptr<Account> account) {
    accounts_[account->account_number] = account;
  }

  Account * get_account(int account_number) {
    if (accounts_.count(account_number)) {
      return accounts_[account_number].get();
    } else {
      return nullptr;
    }
  }

private:
  unordered_map<int, shared_ptr<Account>> accounts_;
};



struct Transaction {

  virtual void execute(Account &) = 0;
};


struct Deposit : public Transaction {

  Deposit(int amount)
    :amount_(amount)
  {}

  void execute(Account & account) {
    account.balance += amount_;
  }

private:
  int amount_;
};

struct Withdraw : public Transaction {

  Withdraw(int amount)
    :amount_(amount)
  {}

  virtual void execute(Account & account) {
    account.balance -= amount_;
  }

private:
  int amount_;
};


class ATM {
public:
  ATM(BankAccounts & bank_accounts)
    :bank_accounts_(bank_accounts)
    ,current_account_(-1)
  {

  }


  void login(int account_number) {
    if (bank_accounts_.get_account(account_number)) {
      this->current_account_ = account_number;
    }
  }


  void logout() {
    current_account_ = -1;
  }

  void execute(Transaction & transaction) {
    if (current_account_ != -1) {
      Account * p = bank_accounts_.get_account(this->current_account_);
      transaction.execute(*p);
    }
  }
private:
  BankAccounts & bank_accounts_;
  int current_account_;
};




int main(int argc, char *argv[])
{
  shared_ptr<User> user1 = make_shared<User>("user1");
  shared_ptr<Account>  account1 = make_shared<Account>(*user1, Account::Kind::CHECKING, 100);


  BankAccounts bank_accounts;
  bank_accounts.add_account(account1);


  ATM atm(bank_accounts);
  atm.login(account1->account_number);
  Deposit deposit(10);

  atm.execute(deposit);

  assert(110 == account1->balance);

  Withdraw Withdraw(20);

  atm.execute(Withdraw);

  assert(90 == account1->balance);



  return 0;
}
