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
