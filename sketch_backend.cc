#include "sketch_backend.h"
#include "util.h"

#include "third_party/temp_file.hh"
#include "third_party/system_runner.hh"
#include "third_party/assert_exception.h"

#include "clang_utility_functions.h"
#include "set_idioms.h"

using namespace clang;

static const std::string sketch_struct = ""
"struct StateResult {\n"
"  int result_state_1;\n"
"  int result_state_2;\n"
"}\n";

static const std::string sketch_return_epilogue =""
"  StateResult ret = new StateResult();\n"
"  ret.result_state_1 = state_1;\n"
"  ret.result_state_2 = state_2;\n"
"  return ret;\n"
"";

static const std::string spec_args = "(int state_1, int state_2, int pkt_1, int pkt_2, int pkt_3, int pkt_4, int pkt_5)";

static const std::string sketch_harness =""
"harness void main" + spec_args + " {\n"
"  StateResult spec_result = codelet(state_1, state_2, pkt_1, pkt_2, pkt_3, pkt_4, pkt_5);\n"
"\n"
"  StateResult impl_result = atom_template(state_1, state_2, pkt_1, pkt_2, pkt_3, pkt_4, pkt_5);\n"
"\n"
"  // Assert values\n"
"  assert(spec_result.result_state_1 == impl_result.result_state_1);\n"
"  assert(spec_result.result_state_2 == impl_result.result_state_2);\n"
"}\n"
"";

std::string sketch_backend_transform(const TranslationUnitDecl * tu_decl) {
  for (const auto * child_decl : dyn_cast<DeclContext>(tu_decl)->decls()) {
    // Transform only packet functions into SKETCH specifications
    if (isa<FunctionDecl>(child_decl) and
        (is_packet_func(dyn_cast<FunctionDecl>(child_decl))) and
        (not collect_state_vars(dyn_cast<FunctionDecl>(child_decl)->getBody()).empty())) {
      std::string sketch_contents = sketch_struct +
                                    file_to_str(std::string(getenv("ATOM_TEMPLATE"))) +
                                    create_sketch_spec((dyn_cast<FunctionDecl>(child_decl)->getBody()), "codelet") +
                                    sketch_harness;
      TempFile sketch_temp_file("/tmp/sketch", ".sk");
      sketch_temp_file.write(sketch_contents);
      try {
        run({"/home/anirudh/sketch-1.6.9/sketch-frontend/sketch", "--bnd-inbits=32", "--bnd-cbits=5", sketch_temp_file.name()});
      } catch (const std::exception & e) {
        std::cerr << "Running sketch failed on the input\n" << sketch_contents << "\n";
        throw std::logic_error("Sketch failed to find a configuration");
      }
    }
  }
  // All the work is done by sketch, this is just perfunctory
  // to make sure the domino top-level doesn't complain about not
  // returning a string.
  return "DONE";
}

std::string create_sketch_spec(const Stmt * function_body, const std::string & spec_name) {
  // Map for renames when creating sketch spec
  std::map<std::string, std::string> rename_map;

  // Collect all state variables seen in the function body
  std::set<std::string> state_refs = collect_state_vars(function_body);

  assert(not state_refs.empty());

  // Create unique names for all state variables
  uint8_t count = 0;
  // Create a set of state variable arguments to construct the sketch spec signature
  for (const auto & state_ref : state_refs) {
    rename_map[state_ref] = "state_" + std::to_string(++count);
  }


  // Store all MemberExpr from the function_body.
  // This gives us all packet fields seen in function_body
  std::set<PktField> all_pkt_fields = gen_var_list(function_body,
                                                   {{VariableType::PACKET, true},
                                                    {VariableType::STATE_SCALAR, false},
                                                    {VariableType::STATE_ARRAY, false}});

  // Store all fields that are written into,
  // i.e. anything that occurs on the lhs
  std::set<PktField> defined_fields;
  for (const auto * stmt : function_body->children()) {
    assert_exception(isa<BinaryOperator>(stmt));
    const auto * bin_op = dyn_cast<BinaryOperator>(stmt);
    assert_exception(bin_op->isAssignmentOp());
    if (isa<MemberExpr>(bin_op->getLHS())) {
      defined_fields.emplace(clang_stmt_printer(dyn_cast<MemberExpr>(bin_op->getLHS())));
    }
  }

  // Store all fields that are part of array subscripts
  // i.e. anything that occurs within square brackets
  std::set<PktField> array_fields;
  for (const auto * stmt : function_body->children()) {
    assert_exception(isa<BinaryOperator>(stmt));
    const auto * bin_op = dyn_cast<BinaryOperator>(stmt);
    assert_exception(bin_op->isAssignmentOp());
    // ArraySubscriptExprs show up directly on the LHS or RHS
    // They don't show up as part of expression because they
    // are hoisted out of code in the stateful flanks pass.
    if (isa<ArraySubscriptExpr>(bin_op->getRHS())) {
      array_fields.emplace(clang_stmt_printer(dyn_cast<ArraySubscriptExpr>(bin_op->getRHS())->getIdx()));
    } else if (isa<ArraySubscriptExpr>(bin_op->getLHS())) {
      array_fields.emplace(clang_stmt_printer(dyn_cast<ArraySubscriptExpr>(bin_op->getLHS())->getIdx()));
    }
  }

  // Subtract defined_fields and array_fields from all_pkt_fields to get incoming fields
  // that are set by someone else before getting into this function body
  std::set<PktField> incoming_fields = all_pkt_fields - defined_fields - array_fields;

  // Create unique names for all fields in incoming_fields.
  count = 0;
  for (const auto & field : incoming_fields) {
    rename_map[field] = "pkt_" + std::to_string(++count);
  }

  // Conjoin object name with field to create scalar variables
  // for all packet fields that are defined in the body
  // i.e. change p.x to p_x
  // Also track the names of all defined packet fields to create
  // declarations for them later (the set defined_vars does this)
  std::set<ScalarVarName> defined_vars;
  for (const auto & field : defined_fields) {
    std::string conjoined_field = field;
    std::replace(conjoined_field.begin(), conjoined_field.end(), '.', '_');
    rename_map[field] = conjoined_field;
    defined_vars.emplace(conjoined_field);
  }

  // Rename any predicate field to predicate field != 0,
  // otherwise Sketch will whine
  for (const auto * stmt : function_body->children()) {
    assert_exception(isa<BinaryOperator>(stmt));
    const auto * bin_op = dyn_cast<BinaryOperator>(stmt);
    assert_exception(bin_op->isAssignmentOp());
    if (isa<ConditionalOperator>(bin_op->getRHS())) {
      const auto * cond_op = dyn_cast<ConditionalOperator>(bin_op->getRHS());
      assert_exception(isa<MemberExpr>(cond_op->getCond()->IgnoreParenImpCasts()));
      const auto * member_op  = dyn_cast<MemberExpr>(cond_op->getCond()->IgnoreParenImpCasts());
      assert_exception(rename_map.find(clang_stmt_printer(member_op)) != rename_map.end());
      const auto old_value = rename_map.at(clang_stmt_printer(member_op));
      rename_map.at(clang_stmt_printer(member_op)) = "(" + old_value + " != 0)";
    }
  }

  // Add function signature
  // TODO: We are hard coding a function signature that includes
  // two state variables and five packet variables because
  // that should suffice for everything we need.
  // Worst case: something goes unused, which shouldn't affect correctness.
  std::string sketch_spec_signature = "StateResult " +
                                      spec_name +
                                      spec_args;

  // Add declarations for defined packet fields
  std::string declaration_stub = "";
  for (const auto & var : defined_vars) {
    declaration_stub += "int " + var + ";" + "\n";
  }

  // Now, go through the function_body, renaming everything in the process.
  std::string sketch_spec_body = "";
  for (const auto * child : function_body->children()) {
    assert_exception(isa<BinaryOperator>(child));
    const auto * bin_op = dyn_cast<BinaryOperator>(child);
    assert_exception(bin_op->isAssignmentOp());
    sketch_spec_body += replace_vars(bin_op->getLHS(), rename_map,
                                     {{VariableType::STATE_SCALAR, true}, {VariableType::STATE_ARRAY, true}, {VariableType::PACKET, true}})
                        + "=" +
                        replace_vars(bin_op->getRHS(), rename_map,
                                    {{VariableType::STATE_SCALAR, true}, {VariableType::STATE_ARRAY, true}, {VariableType::PACKET, true}})
                        + ";\n";
  }

  return sketch_spec_signature + "{" + declaration_stub + sketch_spec_body +  sketch_return_epilogue + "}\n";
}

std::set<std::string> collect_state_vars(const Stmt * stmt) {
  // Recursively scan stmt to generate a set of strings representing
  // either packet fields or state variables used within stmt
  assert_exception(stmt);
  std::set<std::string> ret;
  if (isa<CompoundStmt>(stmt)) {
    for (const auto & child : stmt->children()) {
      ret = ret + collect_state_vars(child);
    }
    return ret;
  } else if (isa<BinaryOperator>(stmt)) {
    const auto * bin_op = dyn_cast<BinaryOperator>(stmt);
    return collect_state_vars(bin_op->getLHS()) + collect_state_vars(bin_op->getRHS());
  } else if (isa<ConditionalOperator>(stmt)) {
    const auto * cond_op = dyn_cast<ConditionalOperator>(stmt);
    return collect_state_vars(cond_op->getCond()) + collect_state_vars(cond_op->getTrueExpr()) + collect_state_vars(cond_op->getFalseExpr());
  } else if (isa<MemberExpr>(stmt)) {
    return std::set<std::string>();
  } else if (isa<DeclRefExpr>(stmt)) {
    return std::set<std::string>{clang_stmt_printer(stmt)};
  } else if (isa<ArraySubscriptExpr>(stmt)) {
    // We return array name here, not array name subscript index.
    return std::set<std::string>{clang_stmt_printer(stmt)};
  } else if (isa<IntegerLiteral>(stmt) or isa<NullStmt>(stmt)) {
    return std::set<std::string>();
  } else if (isa<ParenExpr>(stmt)) {
    return collect_state_vars(dyn_cast<ParenExpr>(stmt)->getSubExpr());
  } else if (isa<UnaryOperator>(stmt)) {
    const auto * un_op = dyn_cast<UnaryOperator>(stmt);
    assert_exception(un_op->isArithmeticOp());
    const auto opcode_str = std::string(UnaryOperator::getOpcodeStr(un_op->getOpcode()));
    assert_exception(opcode_str == "!");
    return collect_state_vars(un_op->getSubExpr());
  } else if (isa<ImplicitCastExpr>(stmt)) {
    return collect_state_vars(dyn_cast<ImplicitCastExpr>(stmt)->getSubExpr());
  } else if (isa<CallExpr>(stmt)) {
    const auto * call_expr = dyn_cast<CallExpr>(stmt);
    std::set<std::string> ret;
    for (const auto * child : call_expr->arguments()) {
      const auto child_uses = collect_state_vars(child);
      ret = ret + child_uses;
    }
    return ret;
  } else {
    throw std::logic_error("collect_state_vars cannot handle stmt of type " + std::string(stmt->getStmtClassName()));
  }
}
