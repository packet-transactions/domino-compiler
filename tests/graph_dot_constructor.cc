#include "../graph.h"

#include <string>
#include <iostream>

#include "gtest.h"

TEST(GraphDotTest, Simple) {
  Graph<void*> g("./graph.dot");

  // Print original
  std::cerr << g << std::endl;
}
