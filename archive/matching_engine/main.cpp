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



using OrderId = std::string;
using Price = uint32_t;
using Shares = uint32_t;


enum class Side: char
{
  Buy = 'B',
  Sell = 'S'
};



enum class OrderType: char
{
  GFD = 'G',
  IOC = 'I'
};

constexpr bool
is_buy(Side s)
{
  return s == Side::Buy;
}

struct SimpleOrder {
  OrderId order_id;
  OrderType order_type;
  Side side;
  Price price = 0;
  Shares shares = 0;


  SimpleOrder (OrderId order_id, OrderType order_type, Side side, Price price, Shares shares)
    :order_id(order_id)
     ,order_type(order_type)
     ,side(side)
     ,price(price)
     ,shares(shares)
  {

  }

  void cancel(Shares canceled_shares) {
    assert (shares >= canceled_shares);
    shares -= canceled_shares;
  }

  void execute(Shares executed_shares) {
    assert (shares >= executed_shares);
    shares -= executed_shares;
  }

  bool done() const {
    return shares == 0;
  }
};

#ifdef __UNITTEST__
TEST (SimpleOrder, basic)
{
  SimpleOrder order = {"order1", OrderType::GFD, Side::Sell, 1000, 100};

  EXPECT_FALSE(order.done());
  order.cancel(50);
  EXPECT_FALSE(order.done());
  order.cancel(50);
  EXPECT_TRUE(order.done());
}
#endif

using OnTradeHandler = std::function<void(const SimpleOrder &, const SimpleOrder &, Shares)>;

struct PriceLevel {
  int total_shares = 0;

  using OrderList = std::list<SimpleOrder>;
  using OrderMap = std::unordered_map<OrderId, OrderList::iterator>;
  using DoneOrders = std::vector<OrderId>;

  void add_order(SimpleOrder order) {
    orders_.push_back(order);
    orders_map_[order.order_id] = std::prev(end(orders_));
    this->total_shares += order.shares;
  }

  void cancel_order(OrderId order_id) {
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

  DoneOrders match(SimpleOrder &order, OnTradeHandler & on_trade) {
    DoneOrders done_orders;
    while (!orders_.empty() && !order.done()) {
      auto to_match = orders_.front();
      assert (order.side != to_match.side);
      auto quantity = std::min(order.shares, to_match.shares);
      order.execute(quantity);
      to_match.execute(quantity);
      on_trade(to_match, order, quantity);
      this->total_shares -= quantity;
      if (to_match.done()) {
        done_orders.push_back(to_match.order_id);
        this->orders_map_.erase(to_match.order_id);
        orders_.pop_front();
      }
    }
    return done_orders;
  }

private:
  OrderList orders_;
  OrderMap orders_map_;
};

#ifdef __UNITTEST__
TEST (PriceLevel, basic)
{
  PriceLevel price_level;
  price_level.add_order(SimpleOrder("order1", OrderType::GFD, Side::Buy, 1000, 10));
  EXPECT_EQ(10, price_level.total_shares);
}
#endif

class DepthBook {
public:
  using LevelInfo = std::pair<Price, Shares>;
  using DoneOrders = std::vector<OrderId>;

  DepthBook(OnTradeHandler handler)
    :on_trade_handler_(handler) {}


  void add_order(OrderId order_id, Side side, OrderType order_type, Price price, Shares shares) {
    if (order_id.empty()) return;
    SimpleOrder order = {order_id, order_type, side, price, shares};
    auto done_orders = this->match(order);
    this->cleanup_done_orders(done_orders);
    if (!order.done() && order.order_type != OrderType::IOC) {
      if (is_buy(side))
        bid_levels_[price].add_order(order);
      else
        ask_levels_[price].add_order(order);
      this->order_to_price_level_map_[order_id] = std::make_pair(order.side, order.price);
    }
  }

  void modify_order(OrderId order_id, Side side, Price price, Shares shares) {
    if (this->exists(order_id)) {
      this->cancel_order(order_id);
      this->add_order(order_id, side, OrderType::GFD, price, shares);
    }
  }

  bool exists(OrderId order_id) {
    return this->order_to_price_level_map_.count(order_id);
  }

  void cancel_order(OrderId order_id) {
    if (this->exists(order_id)) {
      auto v = this->order_to_price_level_map_[order_id];
      auto side = v.first;
      auto price = v.second;
      if (is_buy(side)) {
        if (this->bid_levels_.count(price)) {
          this->bid_levels_[price].cancel_order(order_id);
          if (this->bid_levels_[price].empty()) {
            this->bid_levels_.erase(price);
          }

        }
      } else {
        if (this->ask_levels_.count(price)) {
          this->ask_levels_[price].cancel_order(order_id);
          if (this->ask_levels_[price].empty()) {
            this->ask_levels_.erase(price);
          }

        }
      }
      this->order_to_price_level_map_.erase(order_id);
    }
  }


  // returns done order ids as a result of matching
  DoneOrders match(SimpleOrder &order) {
    DoneOrders done_orders;
    while (is_crossing_with(order) && !order.done()) {
      if (is_buy(order.side)) {
        auto p = begin(this->ask_levels_);
        auto removed = p->second.match(order, this->on_trade_handler_);
        done_orders.insert(done_orders.end(), removed.begin(), removed.end());
        if (p->second.total_shares == 0) {
          this->ask_levels_.erase(p);
        }
      }
      else {
        auto p = begin(this->bid_levels_);
        auto removed = p->second.match(order, this->on_trade_handler_);
        done_orders.insert(done_orders.end(), removed.begin(), removed.end());
        if (p->second.total_shares == 0) {
          this->bid_levels_.erase(p);
        }
      }
    }
    return done_orders;
  }

  bool is_crossing_with(const SimpleOrder & order) {
    if (is_buy(order.side)) {
      return order.price >= this->top_of_ask().first;
    } else {
      return this->top_of_bid().first >= order.price;
    }
  }


  LevelInfo top_of_bid() const {
    return this->level_of_bid(0);
  }

  LevelInfo top_of_ask() const {
    return this->level_of_ask(0);
  }


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


  void reset() {
    this->ask_levels_.clear();
    this->bid_levels_.clear();
    this->order_to_price_level_map_.clear();
  }

  friend std::ostream & operator<< (std::ostream & os, const DepthBook & depth_book);

private:

  void cleanup_done_orders(const std::vector<OrderId> & done_orders) {
    for (auto oid : done_orders) {
      this->order_to_price_level_map_.erase(oid);
    }
  }
  using AskSide = std::map<Price, PriceLevel>;
  using BidSide = std::map<Price, PriceLevel, std::greater<Price>>;
  using OrderToPriceLevelMap = std::unordered_map<OrderId, std::pair<Side, Price>>;


  // price level to order map
  AskSide ask_levels_;
  BidSide bid_levels_;

  // order id to price level map
  // to facilitate locating order in the case
  // of cancael/modify
  OrderToPriceLevelMap order_to_price_level_map_;

  // on match callback
  OnTradeHandler on_trade_handler_;
};

std::ostream & operator<< (std::ostream & os, const DepthBook & depth_book) {
  os << "SELL:" << std::endl;
  for (auto p = depth_book.ask_levels_.rbegin(); p != depth_book.ask_levels_.rend(); p++) {
    os << p->first << " " << p->second.total_shares << std::endl;
  }
  os << "BUY:" << std::endl;
  for (auto p = begin(depth_book.bid_levels_); p != end(depth_book.bid_levels_); p++) {
    os << p->first << " " << p->second.total_shares << std::endl;
  }

  return os;
}

#ifdef __UNITTEST__
TEST(DepthBook, basic)
{
  std::vector< std::tuple<OrderId, OrderId, Shares> > trades;
  auto on_trade = [&trades](const SimpleOrder & o1, const SimpleOrder & o2, Shares shares) {
    trades.emplace_back(o1.order_id, o2.order_id, shares);
  };
  DepthBook book(on_trade);
  book.add_order("order1", Side::Buy, OrderType::GFD, 1000, 10);
  EXPECT_EQ(1000, book.top_of_bid().first);
  book.add_order("order2", Side::Buy, OrderType::GFD, 1001, 10);
  EXPECT_EQ(1001, book.top_of_bid().first);
  book.add_order("order3", Side::Sell, OrderType::GFD, 1003, 10);
  EXPECT_EQ(1003, book.top_of_ask().first);
  book.add_order("order4", Side::Sell, OrderType::GFD, 1002, 10);
  EXPECT_EQ(1002, book.top_of_ask().first);

  EXPECT_EQ(2, book.depth_of_ask());
  EXPECT_EQ(2, book.depth_of_bid());
  book.add_order("order5", Side::Sell, OrderType::IOC, 1004, 10);
  // ioc order does not contribute to book
  EXPECT_EQ(2, book.depth_of_ask());
  book.add_order("order6", Side::Sell, OrderType::GFD, 1004, 10);
  EXPECT_EQ(3, book.depth_of_ask());
  // match order 7 with order 2
  book.add_order("order7", Side::Sell, OrderType::GFD, 1001, 10);
  EXPECT_EQ(1, trades.size());
  EXPECT_EQ((std::tuple<OrderId, OrderId, Shares>{"order2", "order7", 10}), trades[0]);
  // bid becomes 1
  EXPECT_EQ(1, book.depth_of_bid());
  // ask doesn't change
  EXPECT_EQ(3, book.depth_of_ask());

  book.add_order("order8", Side::Sell, OrderType::GFD, 1000, 15);
  EXPECT_EQ(2, trades.size());
  EXPECT_EQ((std::tuple<OrderId, OrderId, Shares>{"order1", "order8", 10}), trades[1]);


  EXPECT_EQ(0, book.depth_of_bid());
  EXPECT_EQ(4, book.depth_of_ask());
  EXPECT_EQ (1000, book.top_of_ask().first);


  book.add_order("order9", Side::Buy, OrderType::GFD, 1001, 20);
  EXPECT_EQ(3, trades.size());
  EXPECT_EQ((std::tuple<OrderId, OrderId, Shares>{"order8", "order9", 5}), trades[2]);
  // matches 1000 and 1001 level
  EXPECT_EQ(1, book.depth_of_bid());
  EXPECT_EQ(3, book.depth_of_ask());

  book.reset();

  // test cancel
  book.add_order("order1", Side::Sell, OrderType::GFD, 1000, 10);
  book.add_order("order2", Side::Buy, OrderType::GFD, 999, 10);
  EXPECT_EQ(1, book.depth_of_ask());
  EXPECT_EQ(1, book.depth_of_bid());
  book.cancel_order("order1");
  book.cancel_order("order2");
  EXPECT_EQ(0, book.depth_of_ask());
  EXPECT_EQ(0, book.depth_of_bid());

  book.reset();
  trades.clear();

  // test order
  book.add_order("order1", Side::Buy, OrderType::GFD, 1000, 10);
  book.add_order("order2", Side::Buy, OrderType::GFD, 1000, 10);
  book.add_order("order3", Side::Sell, OrderType::GFD, 900, 20);
  EXPECT_EQ((std::tuple<OrderId, OrderId, Shares>{"order1", "order3", 10}), trades[0]);
  EXPECT_EQ((std::tuple<OrderId, OrderId, Shares>{"order2", "order3", 10}), trades[1]);

  book.reset();
  trades.clear();

  // test modify
  book.add_order("order1", Side::Buy, OrderType::GFD, 1000, 10);
  book.add_order("order2", Side::Buy, OrderType::GFD, 1000, 10);
  EXPECT_EQ(1, book.depth_of_bid());
  book.modify_order("order1", Side::Buy, 1000, 20);
  book.add_order("order3", Side::Sell, OrderType::GFD, 900, 20);
  EXPECT_EQ(2, trades.size());
  EXPECT_EQ((std::tuple<OrderId, OrderId, Shares>{"order2", "order3", 10}), trades[0]);
  EXPECT_EQ((std::tuple<OrderId, OrderId, Shares>{"order1", "order3", 10}), trades[1]);


}
#endif

class MessageHandler {
public:
  MessageHandler(DepthBook & book):
    book_(book)
  {}


  void handle(const std::string & msg) {
    std::istringstream iss(msg);
    std::string command;
    iss >> command;
    if (command == "BUY" or command == "SELL") {
      handle_add_order(command[0], iss);
    }
    else if (command == "MODIFY")  {
      handle_modify_order(iss);
    }
    else if (command == "CANCEL") {
      handle_cancel_order(iss);
    }
    else if (command == "PRINT") {
      handle_print_book();
    } else {
      handle_other();
    }
  }

private:
  void handle_add_order(char side, std::istringstream & iss) {
    std::string order_type;
    OrderId order_id;
    int price, shares;
    iss >> order_type >> price >> shares >> order_id;
    if (price <= 0 or shares <= 0) return;
    this->book_.add_order(order_id, static_cast<Side>(side), static_cast<OrderType>(order_type[0]), price, shares);
  }

  void handle_modify_order(std::istringstream & iss) {
    std::string order_id, side;
    int price, shares;
    iss >> order_id >> side >> price >> shares;
    if (price <= 0 or shares <= 0) return;
    this->book_.modify_order(order_id, static_cast<Side>(side[0]), price, shares);
  }

  void handle_cancel_order(std::istringstream & iss) {
    std::string order_id;
    iss >> order_id;
    this->book_.cancel_order(order_id);
  }


  void handle_print_book() {
    std::cout << this->book_;
  }


  void handle_other() {
    // no-ops
  }

private:
  DepthBook & book_;
};

#ifdef __UNITTEST__
TEST(MessageHandler, basic)
{
  std::vector< std::tuple<OrderId, OrderId, Shares> > trades;
  auto on_trade = [&trades](const SimpleOrder & o1, const SimpleOrder & o2, Shares shares) {
    trades.emplace_back(o1.order_id, o2.order_id, shares);
  };

  // problem example 1
  DepthBook book(on_trade);
  MessageHandler handler(book);
  handler.handle("BUY GFD 1000 10 order1");
  handler.handle("PRINT");
  EXPECT_EQ(1, book.depth_of_bid());
  EXPECT_EQ(1000, book.top_of_bid().first);
  EXPECT_EQ(10, book.top_of_bid().second);

  // problem example 2
  book.reset();
  handler.handle("BUY GFD 1000 10 order1");
  handler.handle("BUY GFD 1000 20 order2");
  handler.handle("PRINT");
  EXPECT_EQ(1, book.depth_of_bid());
  EXPECT_EQ(1000, book.top_of_bid().first);
  EXPECT_EQ(30, book.top_of_bid().second);

  // problem example 3
  book.reset();
  handler.handle("BUY GFD 1000 10 order1");
  handler.handle("BUY GFD 1001 20 order2");
  handler.handle("PRINT");
  EXPECT_EQ(2, book.depth_of_bid());
  EXPECT_EQ(1001, book.top_of_bid().first);
  EXPECT_EQ(20, book.top_of_bid().second);

  // problem example 4
  book.reset();
  handler.handle("BUY GFD 1000 10 order1");
  handler.handle("SELL GFD 900 20 order2");
  handler.handle("PRINT");
  EXPECT_EQ(0, book.depth_of_bid());
  EXPECT_EQ(1, book.depth_of_ask());
  EXPECT_EQ(1, trades.size());
  EXPECT_EQ((std::tuple<OrderId, OrderId, Shares>{"order1", "order2", 10}), trades[0]);

  // problem example 5
  book.reset();
  trades.clear();
  handler.handle("BUY GFD 1000 10 order1");
  handler.handle("BUY GFD 1010 10 order2");
  handler.handle("SELL GFD 1000 15 order3");
  EXPECT_EQ(2, trades.size());
  EXPECT_EQ((std::tuple<OrderId, OrderId, Shares>{"order2", "order3", 10}), trades[0]);
  EXPECT_EQ((std::tuple<OrderId, OrderId, Shares>{"order1", "order3", 5}), trades[1]);

  // test modify
  book.reset();
  trades.clear();
  handler.handle("BUY GFD 1000 10 order1");
  handler.handle("BUY GFD 1000 10 order2");
  handler.handle("MODIFY order1 BUY 1000 20");
  handler.handle("SELL GFD 900 20 order3");
  EXPECT_EQ(2, trades.size());
  EXPECT_EQ((std::tuple<OrderId, OrderId, Shares>{"order2", "order3", 10}), trades[0]);
  EXPECT_EQ((std::tuple<OrderId, OrderId, Shares>{"order1", "order3", 10}), trades[1]);


  // test ioc order
  book.reset();
  trades.clear();
  handler.handle("BUY IOC 1000 10 order1");
  handler.handle("SELL IOC 1000 10 order1");
  // does not contribute to book
  EXPECT_EQ(0, book.depth_of_bid());
  EXPECT_EQ(0, book.depth_of_ask());
  EXPECT_EQ(0, trades.size());


  // test <0 price/shares and empty order id
  book.reset();
  trades.clear();
  handler.handle("BUY GFD -1000 10 order2");
  EXPECT_EQ(0, book.depth_of_bid());
  EXPECT_EQ(0, book.depth_of_ask());
  handler.handle("BUY GFD 1000 -10 order2");
  EXPECT_EQ(0, book.depth_of_bid());
  EXPECT_EQ(0, book.depth_of_ask());
  handler.handle("BUY GFD 1000 10");
  EXPECT_EQ(0, book.depth_of_bid());
  EXPECT_EQ(0, book.depth_of_ask());
  handler.handle("CANCEL");

  // non exists modify
  book.reset();
  trades.clear();
  handler.handle("BUY GFD 1000 10 order1");
  handler.handle("BUY GFD 1000 5 order2");
  EXPECT_EQ(15, book.top_of_bid().second);
  handler.handle("CANCEL order2");
  EXPECT_EQ(10, book.top_of_bid().second);

  // cancel order id
  book.reset();
  trades.clear();
  handler.handle("MODIFY order1 BUY 1000 20");
  EXPECT_EQ(0, book.depth_of_bid());
  EXPECT_EQ(0, book.depth_of_ask());

}
#endif

int main(int argc, char * argv[])
{

#ifdef __UNITTEST__

  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();

#else

  auto on_trade = [](const SimpleOrder & lhs, const SimpleOrder & rhs, Shares shares) {
    std::cout << "TRADE " << lhs.order_id << " " << lhs.price << " " << shares
         << " " << rhs.order_id << " " << rhs.price << " " << shares << std::endl;
  };
  DepthBook depth_book(on_trade);
  MessageHandler handler(depth_book);


  std::ios_base::sync_with_stdio(false);
  std::string line;
  while (getline(std::cin, line))
  {
    handler.handle(line);
  }


#endif

}
