#include "../expr_flattener_handler.h"

#include <functional>

#include "gtest.h"
#include "../compiler_pass.h"
#include "../clang_utility_functions.h"
#include "../util.h"

using std::placeholders::_1;

void test_expr_flattener(const std::string & input, const std::string & expected_output) {
  auto expr_flattener_pass = FixedPointPass(std::bind(& ExprFlattenerHandler::transform, ExprFlattenerHandler(), _1));
  const auto output = expr_flattener_pass(input);
  std::cerr << "Actual output is " << output << std::endl;
  std::cerr << "Expected output is " << expected_output << std::endl;
  ASSERT_EQ(compare_after_removing_space(expected_output, output), true);
}

TEST(ExprFlattenerTests, Trivial) {
  test_expr_flattener("int a;", "int a;");
}

TEST(ExprFlattenerTests, AlreadyFlat) {
  test_expr_flattener("struct Packet {int x; int y;}; void func(struct Packet p) { p.x = p.y + 1; }", "struct Packet {int x; int y;}; void func(struct Packet p) { p.x = p.y + 1; }");
}

TEST(ExprFlattenerTests, OnePass) {
  test_expr_flattener("struct Packet {int x; int y; int z;}; void func(struct Packet p) { p.x = p.y + 1 + p.z; }", "struct Packet {int x; int y; int z; int tmp0;}; void func(struct Packet p) { p.tmp0 = p.y + 1; p.x = p.tmp0 + p.z;}");
}

TEST(ExprFlattenerTests, TwoPass) {
  test_expr_flattener("struct Packet {int x; int y; int z;}; void func(struct Packet p) { p.x = p.y + 1 + p.z + 1; }", "struct Packet {int x; int y; int z; int tmp0; int tmp1;}; void func (struct Packet p) { p.tmp1 = p.y + 1; p.tmp0 = p.tmp1 + p.z; p.x = p.tmp0 + 1;}");
}
