#include "../if_conversion_handler.h"

#include <functional>

#include "gtest.h"
#include "../compiler_pass.h"
#include "../clang_utility_functions.h"
#include "../util.h"

using std::placeholders::_1;

TEST(IfConverterTests, Trivial) {
  auto if_converter_pass = SinglePass(std::bind(& IfConversionHandler::transform, IfConversionHandler(), _1));
  const auto input = "int a;";
  const auto output = if_converter_pass(input);
  ASSERT_EQ(compare_after_removing_space(output, input), true);
}

TEST(IfConverterTests, NoIfStmt) {
  auto if_converter_pass = SinglePass(std::bind(& IfConversionHandler::transform, IfConversionHandler(), _1));
  const auto input = "int a; struct Packet { int x; }; void func(struct Packet p) { p.x = 2; }";
  const auto output = if_converter_pass(input);
  std::cerr << "output is " << output << std::endl;
  ASSERT_EQ(compare_after_removing_space("int a; struct Packet { int x; }; void func(struct Packet p) { p.x = (1 ? (2) : p.x); }", output), true);
}
