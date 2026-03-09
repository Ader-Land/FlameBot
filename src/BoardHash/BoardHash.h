#pragma once

#include "../Chess/Chess.h"
#include <cstdint>
#include <string>
#include <unordered_map>


using Key = uint64_t;

namespace BoardHash {
enum Bound { BOUND_EXACT = 0, BOUND_LOWER = 1, BOUND_UPPER = 2 };
struct Entry {
  Key key;
  int score;
  int depth;
  Bound bound;
};

const int TABLE_SIZE = 1048576; // 1M entries
extern Entry *Table;

void init();
void clear();
Key generateHash(const Chess::Board &board);

std::string generateID(const Chess::Board &board);
Chess::Board loadID(const std::string &id);
} // namespace BoardHash