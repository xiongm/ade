/*

  This program provides solution to the simple order book problem.


  1. Build instructions:

  g++ -g -Wall -O3 --std=c++14 ./main.cpp -o ./main
  ./main < /path/to/your/input/file


  2. Output for each sample input
>./main < ./script.txt
45.2
200
45.2
50
51.2
500

>./main < ./script1.txt
24.7
22.5
35.1
250
400
200
37.6
450
37.8

   3. problems/concerns
    * to map an order id to the iterator at its price level
    * i had no time to write this documentation


   4. TODOs
    * stringstream is a bad idea for parsing input, need to get rid of it
    * provide a MicroDollar(or NanoDollar) class for price and a constructor
      to take double price
    * accessing price level is still O(logn) which is suboptimal, so the assumptio      is price level is not huge, i haven't quite figured out a better way yet
 */

#ifdef __UNITTEST__
#include <gtest/gtest.h>
#endif


#include <set>
#include <functional>
#include <vector>
#include <string>
#include <iostream>
#include <assert.h>
#include <unordered_map>
#include <memory>
#include <algorithm>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <map>
#include <iterator>
#include <chrono>
#include <list>


namespace spec
{
  const char * ADD = "add";
  const char * MODIFY = "modify";
  const char * DELETE = "remove";
  const char * GET = "get";

  namespace get_type {
    const char *PRICE = "price";
    const char *SIZE = "size";
  }
}

using OrderId = int;
using Price = double;
using MicroDollars = uint64_t;
using Shares = uint32_t;
using Side = char;


constexpr bool
is_buy(Side s)
{
  return s == 'B';
}

struct SimpleOrder {
  OrderId order_id;
  Side side;
  MicroDollars price = 0;
  Shares shares = 0;


  SimpleOrder (OrderId order_id, Side side, MicroDollars price, Shares shares)
    :order_id(order_id)
     ,side(side)
     ,price(price)
     ,shares(shares)
  {

  }


  void modify(Shares new_shares) {
    shares = new_shares;
  }


  void remove() {
    this->cancel(shares);
  }

  void cancel(Shares canceled_shares) {
    assert (shares >= canceled_shares);
    shares -= canceled_shares;
  }

  bool done() const {
    return shares == 0;
  }
};


struct PriceLevel {
  int total_shares = 0;

  using OrderList = std::list<SimpleOrder>;
  using OrderMap = std::unordered_map<OrderId, OrderList::iterator>;

  void add(SimpleOrder order) {
    orders_.push_back(order);
    orders_map_[order.order_id] = std::prev(end(orders_));
    this->total_shares += order.shares;
  }

  void modify(OrderId order_id, Shares new_shares) {
    if (this->orders_map_.count(order_id)) {
      auto p = this->orders_map_[order_id];
      this->total_shares -= p->shares;
      p->shares = new_shares;
      this->total_shares += p->shares;
    }
  }

  void remove(OrderId order_id) {
    if (this->orders_map_.count(order_id)) {
      auto p = this->orders_map_[order_id];
      this->total_shares -= p->shares;
      this->orders_.erase(p);
      this->orders_map_.erase(order_id);
    }
  }

  bool empty() const {
    return this->orders_.empty();
  }

private:
  OrderList orders_;
  OrderMap orders_map_;
};


class OrderBook {
public:
  using LevelInfo = std::pair<Price, Shares>;

  void add(OrderId order_id, Side side, Price order_price, Shares shares) {
    MicroDollars price = order_price * 1000000;
    SimpleOrder order(order_id, side, price, shares);
    if (!order.done()) {
      if (is_buy(side))
        bid_levels_[price].add(order);
      else
        ask_levels_[price].add(order);
      this->order_to_price_level_map_[order_id] = std::make_pair(order.side, order.price);
    }
  }

  void modify(OrderId order_id, Shares new_shares) {
    if (new_shares == 0) this->remove(order_id);
    if (this->exists(order_id)) {
      auto v = this->order_to_price_level_map_[order_id];
      auto side = v.first;
      auto price = v.second;
      if (is_buy(side)) {
        if (this->bid_levels_.count(price)) {
          this->bid_levels_[price].modify(order_id, new_shares);
        }
      } else {
        if (this->ask_levels_.count(price)) {
          this->ask_levels_[price].modify(order_id, new_shares);
        }
      }
    }
  }


  double get_price(char side, int level) {
    if (is_buy(side)) {
      double res = double(this->level_of_bid(level - 1).first) / 1000000;
      return res;
    } else {
      double res = double(this->level_of_ask(level - 1).first) / 1000000;
      return res;
    }
  }

  int get_size(char side, int level) {
    if (is_buy(side)) {
      return this->level_of_bid(level - 1).second;
    } else {
      return this->level_of_ask(level - 1).second;
    }

  }

  bool exists(OrderId order_id) {
    return this->order_to_price_level_map_.count(order_id);
  }

  void remove(OrderId order_id) {
    if (this->exists(order_id)) {
      auto v = this->order_to_price_level_map_[order_id];
      auto side = v.first;
      auto price = v.second;
      if (is_buy(side)) {
        if (this->bid_levels_.count(price)) {
          this->bid_levels_[price].remove(order_id);
          if (this->bid_levels_[price].empty()) {
            this->bid_levels_.erase(price);
          }

        }
      } else {
        if (this->ask_levels_.count(price)) {
          this->ask_levels_[price].remove(order_id);
          if (this->ask_levels_[price].empty()) {
            this->ask_levels_.erase(price);
          }

        }
      }
      this->order_to_price_level_map_.erase(order_id);
    }
  }


  // unit test purpose
  void reset() {
    this->ask_levels_.clear();
    this->bid_levels_.clear();
    this->order_to_price_level_map_.clear();
  }

private:

  // index starts with 0
  LevelInfo level_of_bid(size_t index) const {
    if (index >= this->bid_levels_.size()) {
      return std::make_pair(std::numeric_limits<Price>::min(), 0);
    }
    auto p = begin(this->bid_levels_);
    while (index) {
      p = next(p);
      index--;
    };
    return std::make_pair(p->first, p->second.total_shares);
  }

  // index starts with 0
  LevelInfo level_of_ask(size_t index) const {
    if (index >= this->ask_levels_.size()) {
      return std::make_pair(std::numeric_limits<Price>::max(), 0);
    }
    auto p = begin(this->ask_levels_);
    while (index) {
      p = next(p);
      index--;
    };
    return std::make_pair(p->first, p->second.total_shares);
  }

  size_t depth_of_bid() const {
    return this->bid_levels_.size();
  }

  size_t depth_of_ask() const {
    return this->ask_levels_.size();
  }


  using AskSide = std::map<MicroDollars, PriceLevel>;
  using BidSide = std::map<MicroDollars, PriceLevel, std::greater<MicroDollars>>;
  using OrderToPriceLevelMap = std::unordered_map<OrderId, std::pair<Side, MicroDollars>>;


  // price level to order map
  AskSide ask_levels_;
  BidSide bid_levels_;

  // order id to price level map
  // to facilitate locating order in the case
  // of cancael/modify
  OrderToPriceLevelMap order_to_price_level_map_;

};


class MessageHandler {
public:
  MessageHandler(OrderBook & book):
    book_(book)
  {}


  void handle(const std::string & msg) {
    std::istringstream iss(msg);
    std::string command;
    iss >> command;
    if (command == spec::ADD) {
      handle_add_order(iss);
    }
    else if (command == spec::MODIFY)  {
      handle_modify_order(iss);
    }
    else if (command == spec::DELETE) {
      handle_remove_order(iss);
    }
    else if (command == spec::GET) {
      handle_get(iss);
    } else {
      handle_other();
    }
  }

private:
  void handle_add_order(std::istringstream & iss) {
    OrderId order_id;
    Side side;
    Price price;
    Shares shares;
    iss >> order_id >> side >> price >> shares;
    this->book_.add(order_id, side, price, shares);
  }

  void handle_modify_order(std::istringstream & iss) {
    OrderId order_id;
    Shares new_shares;
    iss >> order_id >> new_shares;
    this->book_.modify(order_id, new_shares);
  }

  void handle_remove_order(std::istringstream & iss) {
    OrderId order_id;
    iss >> order_id;
    this->book_.remove(order_id);
  }


  void handle_get(std::istringstream & iss) {
    std::string get_type;
    char side;
    int level;
    iss >> get_type >> side >> level;
    if (get_type == spec::get_type::PRICE) {
      std::cout << this->book_.get_price(side, level) << std::endl;
    } else if (get_type == spec::get_type::SIZE) {
      std::cout << this->book_.get_size(side, level) << std::endl;
    }
  }


  void handle_other() {
    // no-ops
  }

private:
  OrderBook & book_;
};

#ifdef __UNITTEST__
TEST(MessageHandler, basic)
{
  // initialize test environment
  OrderBook book;
  MessageHandler handler(book);

  // problem statement example 1
  handler.handle("add 1 B 45.2 100");
  handler.handle("modify 1 50");
  handler.handle("get price B 1//this returns 45.2");
  EXPECT_EQ(45.2, book.get_price('B', 1));
  handler.handle("add 2 S 51.4 200");
  handler.handle("add 3 B 45.1 100");
  handler.handle("get size S 1 // this returns 200");
  EXPECT_EQ(200, book.get_size('S', 1));
  handler.handle("add 4 S 51.2 300");
  handler.handle("add 5 S 51.2 200");
  handler.handle("remove 3");
  handler.handle("get price B 1");
  EXPECT_EQ(45.2, book.get_price('B', 1));
  handler.handle("get size B 1");
  EXPECT_EQ(50, book.get_size('B', 1));
  handler.handle("get price S 1");
  EXPECT_EQ(51.2, book.get_price('S', 1));
  handler.handle("get size S 1");
  EXPECT_EQ(500, book.get_size('S', 1));



  book.reset();

  // problem statement example 2
  handler.handle("add 1 B 22.5 100");
  handler.handle("add 2 S 37.8 250");
  handler.handle("add 3 B 24.7 150");
  handler.handle("get price B 1");
  EXPECT_EQ(24.7, book.get_price('B', 1));
  handler.handle("get price B 2");
  EXPECT_EQ(22.5, book.get_price('B', 2));
  handler.handle("modify 3 50");
  handler.handle("add 4 S 35.1 250");
  handler.handle("add 5 S 37.8 150");
  handler.handle("get price S 1");
  EXPECT_EQ(35.1, book.get_price('S', 1));
  handler.handle("remove 3");
  handler.handle("get size S 1");
  EXPECT_EQ(250, book.get_size('S', 1));
  handler.handle("get size S 2");
  EXPECT_EQ(400, book.get_size('S', 2));
  handler.handle("remove 5");
  handler.handle("add 6 S 37.8 150");
  handler.handle("add 7 S 37.6 350");
  handler.handle("add 8 B 24.7 200");
  handler.handle("get size B 1");
  EXPECT_EQ(200, book.get_size('B', 1));
  handler.handle("get price S 2");
  EXPECT_EQ(37.6, book.get_price('S', 2));
  handler.handle("modify 8 150");
  handler.handle("add 9 S 35.1 200");
  handler.handle("add 10 B 22.5 350");
  handler.handle("get size B 2");
  EXPECT_EQ(450, book.get_size('B', 2));
  handler.handle("get price S 3");
  EXPECT_EQ(37.8, book.get_price('S', 3));


  // this will not cause trouble
  handler.handle(" // comment stuff");


}
#endif

int main(int argc, char * argv[])
{

#ifdef __UNITTEST__

  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();

#else

  OrderBook order_book;
  MessageHandler handler(order_book);


  std::ios_base::sync_with_stdio(false);
  std::string line;
  while (getline(std::cin, line))
  {
    handler.handle(line);
  }


#endif

}
