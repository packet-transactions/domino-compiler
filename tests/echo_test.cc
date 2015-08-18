#include "gtest.h"
#include "../compiler_pass.h"
#include "../clang_utility_functions.h"
#include "../util.h"

TEST(EchoTests, Echo) {
  auto echo_pass = SinglePass(clang_decl_printer);
  const std::string input = "int a;";
  const std::string output = echo_pass(input);
  ASSERT_EQ(compare_after_removing_space(output, input), true);
}
