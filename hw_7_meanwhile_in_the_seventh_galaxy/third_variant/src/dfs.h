#pragma clang diagnostic push
#pragma ide diagnostic ignored "bugprone-reserved-identifier"
#ifndef MEANWHILE_IN_THE_SEVENTH_GALAXY_THIRD_VARIANT_DFS_H
#define MEANWHILE_IN_THE_SEVENTH_GALAXY_THIRD_VARIANT_DFS_H

#include <sys/types.h>

#include <type_traits>

#include "graph.h"
#include "type_list.h"
#include "value_list.h"

// Fix performs:
// _Colors[index] = (_Colors[index] == -1) ? color : _Colors[index];
template <ssize_t color, size_t index, typename _Colors>
struct Fix {
 private:
  static constexpr ssize_t old_color = Get<index, _Colors>::value;

  using fixed_type =
      std::conditional<old_color == -1, std::integral_constant<ssize_t, color>,
                       std::integral_constant<ssize_t, old_color>>;

 public:
  using Colors = typename Set<index, fixed_type::type::value, _Colors>::type;
};

template <typename _Graph, typename _Colors, typename... _Edges>
struct Dfs {};

template <typename _Graph, typename _Colors, typename... _Edges>
struct Dfs<_Graph, _Colors, TypeList<_Edges...>> {
 private:
  using CurEdge = typename TypeList<_Edges...>::Head;
  using TailEdges = typename TypeList<_Edges...>::Tail;

  // CurEdge::from here is always colorized
  static constexpr ssize_t from_color = Get<CurEdge::from, _Colors>::value;

  // Store color of CurEdge::to
  using ToVertex =
      typename TypeAt<CurEdge::to, typename _Graph::Vertices>::type;
  static constexpr ssize_t to_color = Get<CurEdge::to, _Colors>::value;

  // Fix CurEdge::to color
  using ToColorized = Fix<from_color, CurEdge::to, _Colors>;

  // If to_color != -1 then ToVertex was colorized, else make DFS on it edges
  using DfsColorized = typename std::conditional<
      to_color != -1, ToColorized,
      Dfs<_Graph, typename ToColorized::Colors, typename ToVertex::Edges>>;

  using DfsColors = typename DfsColorized::type::Colors;

 public:
  // Make DFS on tail edges
  using Colors = typename Dfs<_Graph, DfsColors, TailEdges>::Colors;
};

template <typename _Graph, typename _Colors>
struct Dfs<_Graph, _Colors, NullType> {
  using Colors = _Colors;
};

template <typename Graph, size_t start, size_t end>
struct PathExists {
 private:
  using Vertices = typename Graph::Vertices;
  using InitColors = typename Construct<-1, Length<Vertices>::value>::type;

  using DfsEdges = typename TypeAt<start, Vertices>::type::Edges;
  using DfsColors = typename Fix<start, start, InitColors>::Colors;

 public:
  using Colors = typename Dfs<Graph, DfsColors, DfsEdges>::Colors;

  static constexpr ssize_t color_start = Get<start, Colors>::value;
  static constexpr ssize_t color_end = Get<end, Colors>::value;
  static constexpr bool value = color_start == color_end;
};

#endif  /// MEANWHILE_IN_THE_SEVENTH_GALAXY_THIRD_VARIANT_DFS_H.

#pragma clang diagnostic pop