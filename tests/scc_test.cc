#include "../graph.h"

#include <string>
#include <iostream>

#include "gtest.h"

TEST(SccTest, Simple) {
  Graph<int> g([] (const int & x) { return std::to_string(x); });
  g.add_node(1);
  g.add_node(2);
  g.add_node(3);
  g.add_edge(1, 2);
  g.add_edge(2, 3);
  g.add_edge(3, 1);

  // Add another back edge
  g.add_edge(3, 2);

  // Get SCCs
  auto sccs = g.scc();
  for (const auto & comp : sccs) {
    for (const auto & node : comp) {
      std::cerr << node << ",";
    }
    std::cerr << std::endl;
  }
}
