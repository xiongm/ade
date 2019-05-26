#pragma once

#include <set>
#include "types.hpp"


template <size_t N>
class TopVolumesRank {
private:
  using Info = std::pair<Symbol, Volume>;

  struct rank_comp {
    bool operator()(const Info & lhs, const Info & rhs) const {
      // rank is based on volume
      return lhs.second < rhs.second;
    }
  };

  using VolumesRank = std::set<Info, rank_comp>;

public:
  void update(const Symbol & symbol, uint32_t volume) {
    auto p = this->find(symbol);
    if (p != end(ranks_)) {
      ranks_.erase(p);
    }
    ranks_.insert({symbol, volume});

    if (ranks_.size() > N) {
      // remove the lowest node
      ranks_.erase(begin(ranks_));
    }
  }


  // returns a list of symbol:volumn pair in descending order
  std::vector<Info> dump() const {
    std::vector<Info> res;
    copy(rbegin(ranks_), rend(ranks_), back_inserter(res));
    return res;
  }


private:
  typename VolumesRank::const_iterator find(const Symbol & symbol) const {
    return find_if(begin(ranks_),
        end(ranks_),
        [symbol](const Info & info) {
          return symbol == info.first;
        }
    );
  }
  VolumesRank ranks_;
};

template <size_t N>
std::ostream & operator<< (std::ostream & os, const TopVolumesRank<N> & stats) {
  for (auto s : stats.dump()) {
    os << s.first << std::setw(OUTPUT_WIDTH - s.first.size()) << s.second << std::endl;

  };
}

