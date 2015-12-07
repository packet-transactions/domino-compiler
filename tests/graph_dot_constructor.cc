#include "../graph.h"

#include <string>
#include <iostream>

#include "gtest.h"

TEST(GraphDotTest, Simple) {
  Graph<void*> g("../test");

  // Print original
  std::cerr << g << std::endl;
}
