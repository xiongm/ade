#ifdef __UNITTEST__
#include <gtest/gtest.h>
#endif


#include <set>
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


using OrderId = uint64_t;
using Symbol = std::string;
using Volume = uint32_t;
using Shares = uint32_t;
template <typename TKey, typename TValue>
using MyMap = std::unordered_map<TKey, TValue>;

struct OrderId_Traits {
  using type = OrderId;
};

struct Symbol_Traits {
  using type = Symbol;
};

struct Shares_Traits {
  using type = Shares;
};

namespace spec {
  constexpr char ADD_ORDER_TYPE = 'A';
  constexpr char ORDER_CANCEL_TYPE = 'X';
  constexpr char ORDER_EXECUTED_TYPE = 'E';
  constexpr char TRADE_TYPE = 'P';

  namespace header {
    constexpr size_t MSG_TYPE_OFFSET = 8;
  }
  namespace add {
    constexpr size_t ORDERID_OFFSET = 9;
    constexpr size_t ORDERID_LENGTH = 12;
    constexpr size_t SHARES_OFFSET = 22;
    constexpr size_t SHARES_LENGTH = 6;
    constexpr size_t SYMBOL_OFFSET = 28;
    constexpr size_t SYMBOL_LENGTH = 6;
  }

  namespace cancel {
    constexpr size_t ORDERID_OFFSET = 9;
    constexpr size_t ORDERID_LENGTH = 12;
    constexpr size_t CANCELED_SHARES_OFFSET = 21;
    constexpr size_t CANCELED_SHARES_LENGTH = 6;
  }

  namespace execute {
    constexpr size_t ORDERID_OFFSET = 9;
    constexpr size_t ORDERID_LENGTH = 12;
    constexpr size_t EXECUTED_SHARES_OFFSET = 21;
    constexpr size_t EXECUTED_SHARES_LENGTH = 6;
  }

  namespace trade {
    constexpr size_t SHARES_OFFSET = 22;
    constexpr size_t SHARES_LENGTH = 6;
    constexpr size_t SYMBOL_OFFSET = 28;
    constexpr size_t SYMBOL_LENGTH = 6;
  }
}


// takes care of top 10
template <size_t N>
class TopVolumesRank {
private:
  using Info = std::pair<Symbol, Volume>;

  struct rank_comp {
    bool operator()(const Info & lhs, const Info & rhs) const {
      // rank is based on volume asc, symbol descn in that order
      // so when traverse backwards, it will be, volume desc, symbol asc
      return lhs.second == rhs.second ? lhs.first > rhs.first : lhs.second < rhs.second;
    }
  };

  using VolumesRank = std::set<Info, rank_comp>;

public:
  void update(const Symbol & symbol, uint32_t volume) {
    auto p = this->find(symbol);
    if (p != end(ranks_)) {
      ranks_.erase(p);
    }
    ranks_.insert(make_pair(symbol, volume));

    if (ranks_.size() > N) {
      // eviction
      ranks_.erase(begin(ranks_));
    }
  }


  // returns a list of symbol:volume pair in volume desc/symbol asc order
  std::vector<Info> dump() const {
    std::vector<Info> res;
    std::copy(std::rbegin(ranks_), std::rend(ranks_), std::back_inserter(res));
    return res;
  }

  template <size_t M>
  friend std::ostream & operator<< (std::ostream & os, const TopVolumesRank<M> & stats);

private:
  typename VolumesRank::const_iterator find(const Symbol & symbol) const {
    return std::find_if(std::begin(ranks_),
        std::end(ranks_),
        [symbol](const Info & info) {
          return symbol == info.first;
        }
    );
  }
  VolumesRank ranks_;
};

template <size_t N>
std::ostream & operator<< (std::ostream & os, const TopVolumesRank<N> & stats) {
  for (auto p = std::rbegin(stats.ranks_); p != std::rend(stats.ranks_); p++) {
    os << p->first << std::setw(15 - p->first.size()) << p->second << std::endl;
  }
  return os;
}

#ifdef __UNITTEST__
// unit tests
TEST (TopVolumesRank, basic)
{
  using StatsList = std::vector< std::pair<Symbol, Volume> >;
  TopVolumesRank<3> stats;
  stats.update("AAPL", 1);
  EXPECT_EQ((StatsList{{"AAPL", 1}}), stats.dump());
  stats.update("MSFT", 2);
  EXPECT_EQ((StatsList{{"MSFT", 2}, {"AAPL", 1}}), stats.dump());
  stats.update("IBM", 3);
  EXPECT_EQ((StatsList{{"IBM", 3}, {"MSFT", 2}, {"AAPL", 1}}), stats.dump());
  stats.update("AAPL", 4);
  EXPECT_EQ((StatsList{{"AAPL", 4}, {"IBM", 3}, {"MSFT", 2}}), stats.dump());
  stats.update("AMZN", 5);
  // MSFT is evicted by AMZN
  EXPECT_EQ((StatsList{{"AMZN", 5}, {"AAPL", 4}, {"IBM", 3}}), stats.dump());
  stats.update("MSFT", 2);
  // doesn't change since MSFT:2 is not top 3
  EXPECT_EQ((StatsList{{"AMZN", 5}, {"AAPL", 4}, {"IBM", 3}}), stats.dump());
  // add a case where volume is the same
  stats.update("MSFT", 4);
  EXPECT_EQ((StatsList{{"AMZN", 5}, {"AAPL", 4}, {"MSFT", 4}}), stats.dump());
}

#endif



/*
 * A simplified Order class that ONLY cares about shares
 * since that's what the original problem cares about
 */
struct SimpleOrder {
  Shares shares = 0;
  Shares outstanding_shares = 0;

  SimpleOrder(Shares shares) {
    this->open(shares);
  }

  SimpleOrder() {}

  void open(Shares opened_shares) {
    assert (opened_shares > 0);
    shares = opened_shares;
    outstanding_shares = opened_shares;
  }

  void cancel(Shares canceled_shares) {
    assert (outstanding_shares >= canceled_shares);
    outstanding_shares -= canceled_shares;
  }

  void execute(Shares executed_shares) {
    assert (outstanding_shares >= executed_shares);
    outstanding_shares -= executed_shares;
  }

  bool done() const {
    return shares != 0 && outstanding_shares == 0;
  }
};

#ifdef __UNITTEST__
TEST (SimpleOrder, basic)
{
  {
    SimpleOrder order;
    EXPECT_FALSE(order.done());
    order.open(100);
    EXPECT_FALSE(order.done());
    order.cancel(50);
    EXPECT_FALSE(order.done());
    order.cancel(50);
    EXPECT_TRUE(order.done());
    order.open(100);
    EXPECT_FALSE(order.done());
    order.execute(100);
    EXPECT_TRUE(order.done());
  }
  {
    SimpleOrder order(100);
    order.execute(100);
    EXPECT_TRUE(order.done());
  }
}
#endif

/*
 *
 *  maintains orders table
 *
 */
struct SimpleOrderBook {

  using SharedPtr = std::shared_ptr<SimpleOrderBook>;

  SimpleOrderBook(const Symbol & symbol)
    :symbol_(symbol)
    ,volume_ (0)
  {}

  void add_order(OrderId id, Shares shares) {
    order_book_[id] = SimpleOrder(shares);
  }


  void cancel_order(OrderId id, Shares shares) {
    if (this->has(id)) {
      order_book_[id].cancel(shares);
      this->cleanup(id);
    }
  }

  void execute_order(OrderId id, Shares shares) {
    if (this->has(id)) {
      this->log_trade(shares);
      order_book_[id].execute(shares);
      this->cleanup(id);
    }
  }

  bool has(OrderId id) const {
    return this->order_book_.count(id);
  }


  Symbol symbol() const {
    return this->symbol_;
  }

  Volume volume() const {
    return this->volume_;
  }

  size_t size() const {
    return this->order_book_.size();
  }


  /**
   * this interface is exposed so that
   * caller can log trads not reflected
   * in book, e.g. trade message
   *
   */
  void log_trade(Shares shares) {
    this->volume_ += shares;
  }


private:
  using OrderBook = MyMap<OrderId, SimpleOrder>;
  void cleanup(OrderId id) {
    if (order_book_[id].done()) {
      order_book_.erase(id);
    }
  }
  OrderBook order_book_;
  const Symbol symbol_;
  Volume volume_;
};


#ifdef __UNITTEST__
TEST (SimpleOrderBook, basic)
{
  SimpleOrderBook order_book("AAPL");
  order_book.add_order(1, 100);
  order_book.add_order(2, 200);
  EXPECT_EQ(0, order_book.volume());
  EXPECT_EQ(2, order_book.size());
  order_book.execute_order(2, 50);
  EXPECT_EQ(50, order_book.volume());
  EXPECT_EQ(2, order_book.size());
  order_book.cancel_order(1, 100);
  EXPECT_EQ(1, order_book.size());
  order_book.execute_order(2, 150);
  EXPECT_EQ(0, order_book.size());
  EXPECT_EQ(200, order_book.volume());
  order_book.log_trade(50);
  EXPECT_EQ(250, order_book.volume());
}
#endif


/**
 *
 *
 * maintains a per symbol order book
 *
 *
 */
class Book {
public:

  using StatsType = TopVolumesRank<10>;
  // TODO: try multi_index_container
  using BookLookupTable = MyMap<OrderId, SimpleOrderBook::SharedPtr>;
  using SymbolBookTable = MyMap<Symbol, SimpleOrderBook::SharedPtr>;

  void add_order(OrderId order_id, const Symbol & sym, Shares shares) {
    auto order_book = this->get_order_book(sym);

    order_book->add_order(order_id, shares);
    book_lookup_[order_id] = order_book;

  }

  void cancel_order(OrderId order_id, Shares shares) {
    if (!book_lookup_.count(order_id)) return;
    auto order_book = book_lookup_[order_id];
    order_book->cancel_order(order_id, shares);
    if (!order_book->has(order_id)) {
      book_lookup_.erase(order_id);
    }
  }

  void execute_order(OrderId order_id, Shares shares) {
    if (!book_lookup_.count(order_id)) return;
    auto order_book = book_lookup_[order_id];
    order_book->execute_order(order_id, shares);
    if (!order_book->has(order_id)) {
      book_lookup_.erase(order_id);
    }
    top_ten_.update(order_book->symbol(), order_book->volume());
  }


  void add_trade(const Symbol & sym, Shares shares) {
    auto order_book = this->get_order_book(sym);
    order_book->log_trade(shares);
    top_ten_.update(order_book->symbol(), order_book->volume());
  }


  const StatsType & stats() const {
    return top_ten_;
  }


private:

  SimpleOrderBook::SharedPtr get_order_book(const Symbol & sym) {
    if (!symbol_book_.count(sym)) {
      symbol_book_[sym] = std::make_shared<SimpleOrderBook>(sym);
    }
    return symbol_book_[sym];
  }


  BookLookupTable book_lookup_;
  SymbolBookTable symbol_book_;
  StatsType top_ten_;
};

#ifdef __UNITTEST__
TEST (Book, basic)
{
  using StatsList = std::vector< std::pair<Symbol, Volume> >;
  Book book;
  book.add_order(1, "MSFT", 100);
  book.add_order(2, "IBM", 100);
  book.add_order(3, "AAPL", 100);
  book.execute_order(1, 50);
  book.execute_order(2, 30);
  book.execute_order(3, 20);
  EXPECT_EQ((StatsList{{"MSFT", 50}, {"IBM", 30}, {"AAPL", 20}}), book.stats().dump());
  book.add_order(4, "FB", 100);
  book.execute_order(4, 60);
  EXPECT_EQ((StatsList{{"FB", 60}, {"MSFT", 50}, {"IBM", 30}, {"AAPL", 20}}), book.stats().dump());
  book.add_trade("FB", 100);
  EXPECT_EQ((StatsList{{"FB", 160}, {"MSFT", 50}, {"IBM", 30}, {"AAPL", 20}}), book.stats().dump());
}
#endif




/**
 *
 *
 * helper functions for pitch
 *
 *
 *
 */
namespace parser {
  template<typename T>
  typename T::type parse(const char * start, size_t length);
};


template<>
Shares
parser::parse<Shares_Traits> (const char * start, size_t length) {
  Shares res = 0;
  while(length) {
    res = res * 10 + (*start++ - '0');
    length--;
  }
  return res;
}


template<>
OrderId
parser::parse<OrderId_Traits> (const char * start, size_t length) {
  OrderId res = 0;
  while(length) {
    res = res * 36 +  (*start >= 'A' ? (*start - 'A' + 10) : (*start - '0'));
    start++;
    length--;
  }
  return res;
}

template<>
Symbol
parser::parse<Symbol_Traits> (const char * start, size_t length) {
  Symbol sym;
  while (length && *start != ' ') {
    sym.push_back(*start++);
    length--;
  }
  return sym;
}

class PitchMessageHandler {
public:
  PitchMessageHandler(Book & book):
    book_(book)
  {}


  void handle(const char * msg) {
    char msg_type = *(msg + spec::header::MSG_TYPE_OFFSET);
    switch (msg_type) {
      case spec::ADD_ORDER_TYPE:
        handle_add_order(msg);
        break;
      case spec::ORDER_CANCEL_TYPE:
        handle_order_cancel(msg);
        break;
      case spec::ORDER_EXECUTED_TYPE:
        handle_order_executed(msg);
        break;
      case spec::TRADE_TYPE:
        handle_trade(msg);
        break;
      default:
        handle_other(msg);
    }
  }

private:
  void handle_add_order(const char * msg) {
    using namespace spec::add;
    OrderId oid = parser::parse<OrderId_Traits>(
      msg + ORDERID_OFFSET,
      ORDERID_LENGTH
    );
    Shares shares = parser::parse<Shares_Traits>(
      msg + SHARES_OFFSET,
      SHARES_LENGTH
    );
    Symbol sym = parser::parse<Symbol_Traits>(
      msg + SYMBOL_OFFSET,
      SYMBOL_LENGTH
    );
    this->book_.add_order(oid, sym, shares);

  }

  void handle_order_cancel(const char *msg) {
    using namespace spec::cancel;
    OrderId oid = parser::parse<OrderId_Traits>(
      msg + ORDERID_OFFSET,
      ORDERID_LENGTH
    );
    Shares shares = parser::parse<Shares_Traits>(
      msg + CANCELED_SHARES_OFFSET,
      CANCELED_SHARES_LENGTH
    );
    this->book_.cancel_order(oid, shares);
  }


  void handle_order_executed(const char * msg) {
    using namespace spec::execute;
    OrderId oid = parser::parse<OrderId_Traits>(
      msg + ORDERID_OFFSET,
      ORDERID_LENGTH
    );
    Shares shares = parser::parse<Shares_Traits>(
      msg + EXECUTED_SHARES_OFFSET,
      EXECUTED_SHARES_LENGTH
    );
    this->book_.execute_order(oid, shares);
  }

  void handle_trade(const char * msg) {
    using namespace spec::trade;
    Shares shares = parser::parse<Shares_Traits>(
      msg + SHARES_OFFSET,
      SHARES_LENGTH
    );
    Symbol sym = parser::parse<Symbol_Traits>(
      msg + SYMBOL_OFFSET,
      SYMBOL_LENGTH
    );
    this->book_.add_trade(sym, shares);
  }


  void handle_other(const char * ) {
    // pass
  }

private:
  Book & book_;
};

#ifdef __UNITTEST__
TEST(PitchMessageHandler, basic)
{
  // TODO: need more test cases!
  using StatsList = std::vector< std::pair<Symbol, Volume> >;
  Book book;
  PitchMessageHandler handler(book);
  handler.handle("28800011AAK27GA0000DTS000100SH    0000619200Y");
  handler.handle("28858232XAK27GA0000DT000100");
  handler.handle("28800162A1K27GA00000XB000100AAPL  0001828600Y");
  handler.handle("28800180X1K27GA00000X000100");
  handler.handle("28800180A1K27GA00000XB000100AAPL  0001830100Y");
  handler.handle("28800318E1K27GA00000X00010000001AQ00001");
  EXPECT_EQ((StatsList{{"AAPL", 100}}), book.stats().dump());
}
#endif

int main(int argc, char * argv[])
{

#ifdef __UNITTEST__

  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();

#else
  auto begin = std::chrono::high_resolution_clock::now();


  Book book;
  PitchMessageHandler handler(book);


  std::ios_base::sync_with_stdio(false);
  std::string line;
  while (getline(std::cin, line))
  {
    const char *p = line.c_str();
    // ignore first character per instructions
    handler.handle(p + 1);
  }

  auto end = std::chrono::high_resolution_clock::now();
  auto intake_time = std::chrono::duration_cast<std::chrono::nanoseconds>(end-begin).count() / 1000000;

  std::cout << book.stats();
  std::cout << "Real running time:" << intake_time << " ms" << std::endl;


#endif

}
