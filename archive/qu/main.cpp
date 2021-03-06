/*
This problem will require you to write an application that will read an input file ('input.csv') and write out a new file calculated from the inputs. Please use C++.

We are looking for an easily extendable solution with a strong emphasis on readability, even though it may not seem required for this application. We will evaluate your submission based on the following criteria:
             Readability, Design, and Construction
             Tests
             Use of modern C++14/17 Idioms
             Use of C++ TMP where appropriate
             Correctness

We are big fans of test driven development (TDD) and SOLID design principles. Please aim to spend about an 1-2 hours to complete this exercise. Send your source code and output.csv in a zipped git repo back for evaluation when complete. Include the amount of time you spent working on the solution.

Tips:
-              Use the 2 hour limit as a guiding constraint.
-              Expect that your code will be extended in the future.
-              Send us code you would put in production.

-enjoy!
--Quantlab


_______________________________________________________________________________

Input:
The input file represents a very simplified stream of trades on an exchange.
Each row represents a trade.  If you don't know what that means don't worry.
The data can be thought of as a time series of values in columns:

<TimeStamp>,<Symbol>,<Quantity>,<Price>

Although the provided input file is small, the solution should be able to handle
a source dataset well beyond the amount memory and hard disk space on your machine.

Definitions
- TimeStamp is value indicating the microseconds since midnight.
- Symbol is the 3 character unique identifier for a financial
  instrument (Stock, future etc.)
- Quantity is the amount traded
- Price is the price of the trade for that financial instrument.

Safe Assumptions:
- TimeStamp is always for the same day and won't roll over midnight.
- TimeStamp is increasing or same as previous tick (time gap will never be < 0).
- Price - our currency is an integer based currency.  No decimal points.
- Price - Price is always > 0.

Example: here is a row for a trade of 10 shares of aaa stock at a price of 12
1234567,aaa,10,12

Problem:
Find the following on a per symbol basis:
- Maximum time gap
  (time gap = Amount of time that passes between consecutive trades of a symbol)
  if only 1 trade is in the file then the gap is 0.
- Total Volume traded (Sum of the quantity for all trades in a symbol).
- Max Trade Price.
- Weighted Average Price.  Average price per unit traded not per trade.
  Result should be truncated to whole numbers.

  Example: the following trades
	20 shares of aaa @ 18
	5 shares of aaa @ 7
	Weighted Average Price = ((20 * 18) + (5 * 7)) / (20 + 5) = 15

Output:
Your solution should produce a file called 'output.csv'.
file should be a comma separate file with this format:
<symbol>,<MaxTimeGap>,<Volume>,<WeightedAveragePrice>,<MaxPrice>

The output should be sorted by symbol ascending ('aaa' should be first).

Sample Input:
52924702,aaa,13,1136
52924702,aac,20,477
52925641,aab,31,907
52927350,aab,29,724
52927783,aac,21,638
52930489,aaa,18,1222
52931654,aaa,9,1077
52933453,aab,9,756

Sample Output:
aaa,5787,40,1161,1222
aab,6103,69,810,907
aac,3081,41,559,638

Send your source code and output.csv in a zipped git repo back for evaluation when complete.
Include the amount of time you spent working on the solution.

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
#include <list>


using Timestamp = uint64_t;
using Symbol = std::string;
using Shares = uint64_t;
using Volumes = uint64_t;
using Price = uint64_t;


struct SymbolStats {

  //using handler=SymbolStatsHandler;

  Symbol symbol;
  Timestamp last_timestamp = 0;
  Timestamp max_time_gap = 0;
  Shares volumes;
  Price total_price;
  Price max_price = 0;


  Price average_price() const {
    return volumes > 0 ? total_price / volumes : total_price;
  }


};

class SymbolBook {
public:
  using SymbolMap = std::unordered_map<Symbol, SymbolStats>;
  using SymbolOrderMap = std::set<Symbol>;

  void add(Timestamp timestamp, Symbol symbol, Shares shares, Price price) {
    if (!map_.count(symbol)) {
      ordered_map_.insert(symbol);
      map_[symbol] = SymbolStats {symbol, timestamp, 0, shares, shares * price, price};
    } else {
      auto & report = map_[symbol];
      report.max_time_gap = std::max(report.max_time_gap, timestamp - report.last_timestamp);
      report.last_timestamp = timestamp;
      report.volumes += shares;
      report.total_price += shares * price;
      report.max_price = std::max(report.max_price, price);
    }
  }

  friend std::ostream & operator<<(std::ostream &, const SymbolBook &);


  // for unit test
  std::vector<SymbolStats> dump() const {
    std::vector<SymbolStats> res;
    for (auto p = std::begin(ordered_map_); p != std::end(ordered_map_); p++) {
      res.push_back(map_[*p]);
    }
    return res;
  }

private:
  mutable SymbolMap map_;
  SymbolOrderMap ordered_map_;
};

std::ostream & operator<< (std::ostream & os, const SymbolBook & book) {
  std::vector<SymbolStats> dump = book.dump();
  for (auto & report : dump) {
    os << report.symbol << ","
       << report.max_time_gap << ","
       << report.volumes << ","
       << report.average_price() << ","
       << report.max_price << std::endl;
  }
  return os;
}

template<class T>
class MessageHandler {
public:
  MessageHandler(T & symbol_book):
    book_(symbol_book)
  {}


  void handle(const std::string & msg) {
    std::istringstream iss(msg);
    Timestamp timestamp;
    Symbol symbol;
    Shares shares;
    Price price;
    char eater;
    iss >> timestamp >> eater;
    getline(iss, symbol, ',');
    iss >> shares >> eater >> price;
    this->book_.add(timestamp, symbol, shares, price);
  }

private:
  T & book_;
};

#ifdef __UNITTEST__
TEST(MessageHandler, basic)
{
  // initialize test environment
  SymbolBook book;
  MessageHandler<SymbolBook> handler(book);

  // problem statement example 1
  handler.handle("52924702,aaa,13,1136");
  handler.handle("52924702,aac,20,477");
  handler.handle("52925641,aab,31,907");
  handler.handle("52927350,aab,29,724");
  handler.handle("52927783,aac,21,638");
  handler.handle("52930489,aaa,18,1222");
  handler.handle("52931654,aaa,9,1077");
  handler.handle("52933453,aab,9,756");

  auto reports = book.dump();

  EXPECT_EQ(3, reports.size());

  EXPECT_EQ("aaa", reports[0].symbol);
  EXPECT_EQ(5787, reports[0].max_time_gap);
  EXPECT_EQ(40, reports[0].volumes);
  EXPECT_EQ(1161, reports[0].average_price());
  EXPECT_EQ(1222, reports[0].max_price);

  EXPECT_EQ("aab", reports[1].symbol);
  EXPECT_EQ(6103, reports[1].max_time_gap);
  EXPECT_EQ(69, reports[1].volumes);
  EXPECT_EQ(810, reports[1].average_price());
  EXPECT_EQ(907, reports[1].max_price);

  EXPECT_EQ("aac", reports[2].symbol);
  EXPECT_EQ(3081, reports[2].max_time_gap);
  EXPECT_EQ(41, reports[2].volumes);
  EXPECT_EQ(559, reports[2].average_price());
  EXPECT_EQ(638, reports[2].max_price);

}

#endif

int main(int argc, char * argv[])
{

#ifdef __UNITTEST__

  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();

#else

  SymbolBook symbol_book;
  MessageHandler<SymbolBook> handler(symbol_book);


  std::ios_base::sync_with_stdio(false);
  std::string line;
  while (getline(std::cin, line))
  {
    handler.handle(line);
  }

  std::ofstream ofs("./output.csv");

  ofs << symbol_book;


#endif

}
