#include "../strength_reducer.h"

#include <functional>

#include "gtest.h"
#include "../compiler_pass.h"
#include "../clang_utility_functions.h"
#include "../util.h"

void test_strength_reducer(const std::string & input, const std::string & expected_output) {
  auto strength_reducer_pass = SinglePass(strength_reducer_transform);
  const auto output = strength_reducer_pass(input);
  std::cerr << "Actual output is " << output << std::endl;
  std::cerr << "Expected output is " << expected_output << std::endl;
  ASSERT_EQ(compare_after_removing_space(expected_output, output), true);
}

TEST(StrengthReducerTests, Trivial) {
  test_strength_reducer("int a;", "int a;");
}

TEST(StrengthReducerTests, SingleTernary) {
  test_strength_reducer("struct Packet { int x ; }; void func(struct Packet p) { p.x = 1 ? 2 : p.x; }",
                        "struct Packet { int x ; }; void func(struct Packet p) { p.x = 2; }");
}

TEST(StrengthReducerTests, SingleTernaryWithAnd) {
  test_strength_reducer("struct Packet { int x ; int y; }; void func(struct Packet p) { p.x = (1 && p.y) ? 2 : p.x; }",
                        "struct Packet { int x ; int y; }; void func(struct Packet p) { p.x = p.y ? 2 : p.x; }");
}
