#include "../if_conversion_handler.h"

#include <functional>

#include "gtest.h"
#include "../compiler_pass.h"
#include "../clang_utility_functions.h"
#include "../util.h"

using std::placeholders::_1;

void test_if_converter(const std::string & input, const std::string & expected_output) {
  auto if_converter_pass = SinglePass(std::bind(& IfConversionHandler::transform, IfConversionHandler(), _1));
  const auto output = if_converter_pass(input);
  std::cerr << "Actual output is " << output << std::endl;
  std::cerr << "Expected output is " << expected_output << std::endl;
  ASSERT_EQ(compare_after_removing_space(expected_output, output), true);
}

TEST(IfConverterTests, Trivial) {
  test_if_converter("int a;", "int a;");
}

TEST(IfConverterTests, NoIfStmt) {
  test_if_converter("int a; struct Packet { int x; }; void func(struct Packet p) { p.x = 2; }",
                    "int a; struct Packet { int x; }; void func(struct Packet p) { p.x = (1 ? (2) : p.x); }");
}

TEST(IfConverterTests, OneIfStmt) {
  test_if_converter("int a; struct Packet { int x; }; void func(struct Packet p) { if(p.x) p.x = 2; }",
                    "int a; struct Packet { int x; int tmp0; }; void func(struct Packet p) { p.tmp0 = (1 ? (p.x) : p.tmp0); p.x = ((1 && p.tmp0) ? (2) : p.x); }");
}

TEST(IfConverterTests, IfElseStmt) {
  test_if_converter("struct Packet {int x;}; void func(struct Packet p) { if(p.x) p.x = 2; else p.x = 3;}",
                    "struct Packet { int x; int tmp0; }; void func(struct Packet p) { p.tmp0 = (1 ? (p.x) : p.tmp0); p.x = ((1 && p.tmp0) ? (2) : p.x); p.x = ((1 && ! p.tmp0) ? (3) : p.x); }");
}
