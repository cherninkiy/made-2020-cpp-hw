#ifndef MEANWHILE_IN_THE_SEVENTH_GALAXY_FIRST_VARIANT_GAME_H
#define MEANWHILE_IN_THE_SEVENTH_GALAXY_FIRST_VARIANT_GAME_H

#include <sys/types.h>

#include "binary.h"
#include "xor_sum.h"

//////////////////////////////////////////////////////////////////////////////////////////
/// HeapIndex

template <size_t Index, size_t Sum, size_t Head, size_t... Tail>
struct HeapIndex {
  static constexpr ssize_t value =
      (Head - (Sum ^ Head) <= Head) * Index +
      (Head - (Sum ^ Head) > Head) * HeapIndex<Index + 1, Sum, Tail...>::value;
};

template <size_t Index, size_t Sum, size_t Head>
struct HeapIndex<Index, Sum, Head> {
  static constexpr ssize_t value = Index;
};

//////////////////////////////////////////////////////////////////////////////////////////
/// RocksCount

template <size_t Sum, size_t Head, size_t... Tail>
struct RocksCount {
  static constexpr ssize_t value =
      (Head - (Sum ^ Head) <= Head) * (Head - (Sum ^ Head)) +
      (Head - (Sum ^ Head) > Head) * (RocksCount<Sum, Tail...>::value);
};

template <size_t Sum, size_t Head>
struct RocksCount<Sum, Head> {
  static constexpr ssize_t value = Head - (Sum ^ Head);
};

//////////////////////////////////////////////////////////////////////////////////////////
/// Game

template <size_t... Heaps>
struct Game {
  static constexpr size_t sum = XorSum<Heaps...>::value;
  static constexpr bool first_player_wins = (sum != 0);
  static constexpr ssize_t first_move_heap_index =
      (sum != 0) * HeapIndex<0, sum, Heaps...>::value - (sum == 0);
  static constexpr ssize_t first_move_rocks_count =
      (sum != 0) * RocksCount<sum, Heaps...>::value - (sum == 0);
};

#endif  /// MEANWHILE_IN_THE_SEVENTH_GALAXY_FIRST_VARIANT_GAME_H.
