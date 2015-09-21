#include "partitioning.h"

#include "third_party/assert_exception.h"

#include "util.h"
#include "clang_utility_functions.h"
#include "unique_identifiers.h"
#include "set_idioms.h"

using namespace clang;

std::string inst_block_printer(const InstBlock & iblock) {
  std::string ret = "";
  assert_exception(not iblock.empty());
  for (auto & op : iblock) ret += clang_stmt_printer(op) + ";\n";
  return ret;
}

Graph<const BinaryOperator *> handle_state_vars(const std::vector<const BinaryOperator *> & stmt_vector, const Graph<const BinaryOperator*> & dep_graph) {
  Graph<const BinaryOperator*> ret = dep_graph;
  std::map<std::string, const BinaryOperator *> state_reads;
  std::map<std::string, const BinaryOperator *> state_writes;
  for (const auto * stmt : stmt_vector) {
    const auto * lhs = stmt->getLHS()->IgnoreParenImpCasts();
    const auto * rhs = stmt->getRHS()->IgnoreParenImpCasts();
    // At this stage, after stateful_flanks has been run, the only
    // way state variables (scalar or array-based) appear is either on the LHS or on the RHS
    // and they appear by themselves (not as part of another expression)
    // Which is why we don't need to recursively traverse an AST to check for state vars
    if (isa<DeclRefExpr>(rhs) or isa<ArraySubscriptExpr>(rhs)) {
      // Should see exactly one read to a state variable
      assert_exception(state_reads.find(clang_stmt_printer(rhs)) == state_reads.end());
      state_reads[clang_stmt_printer(rhs)] = stmt;
    } else if (isa<DeclRefExpr>(lhs) or isa<ArraySubscriptExpr>(lhs)) {
      // Should see exactly one write to a state variable
      assert_exception(state_writes.find(clang_stmt_printer(lhs)) == state_writes.end());
      state_writes[clang_stmt_printer(lhs)] = stmt;
      const auto state_var = clang_stmt_printer(lhs);
      // Check state_var exists in both maps
      assert_exception(state_reads.find(state_var) != state_reads.end());
      assert_exception(state_writes.find(state_var) != state_writes.end());
      ret.add_edge(state_reads.at(state_var), state_writes.at(state_var));
      ret.add_edge(state_writes.at(state_var), state_reads.at(state_var));
    }
  }

  // Check that there are pairs of reads and writes for every state variable
  for (const auto & pair : state_reads) {
    if (state_writes.find(pair.first) == state_writes.end()) {
      throw std::logic_error(pair.first + " has a read that isn't paired with a write ");
    }
  }
  return ret;
}

bool op_reads_var(const BinaryOperator * op, const Expr * var) {
  assert_exception(op);
  assert_exception(var);

  // We only check packet variables here because handle_state_vars
  // takes care of state variables.
  auto read_vars = gen_var_list(op->getRHS(), {{VariableType::PACKET, true},
                                               {VariableType::STATE_SCALAR, false},
                                               {VariableType::STATE_ARRAY, false}});

  // If the LHS is an array subscript expression, we need to check inside the subscript as well
  if (isa<ArraySubscriptExpr>(op->getLHS())) {
    const auto * array_op = dyn_cast<ArraySubscriptExpr>(op->getLHS());
    const auto read_vars_lhs = gen_var_list(array_op->getIdx(), {{VariableType::PACKET, true},
                                                                {VariableType::STATE_SCALAR, false},
                                                                {VariableType::STATE_ARRAY, false}});
    read_vars = read_vars + read_vars_lhs;
  }

  return (read_vars.find(clang_stmt_printer(var)) != read_vars.end());
}

bool depends(const BinaryOperator * op1, const BinaryOperator * op2) {
  // If op1 succeeds op2 in program order,
  // return false right away
  if (not (op1->getLocStart() < op2->getLocStart())) {
    return false;
  }

  // op1 writes the same variable that op2 writes (Write After Write)
  if (clang_stmt_printer(op1->getLHS()) == clang_stmt_printer(op2->getLHS())) {
    throw std::logic_error("Cannot have Write-After-Write dependencies in SSA form from " + clang_stmt_printer(op1) + " to " + clang_stmt_printer(op2) + "\n");
  }

  // op1 reads a variable that op2 writes (Write After Read)
  if (op_reads_var(op1, op2->getLHS())) {
    // Make an exception for state variables. There is no way around this.
    // There is no need to add this edge, because handle_state_vars() does
    // this already.
    if (isa<DeclRefExpr>(op2->getLHS()) or isa<ArraySubscriptExpr>(op2->getLHS())) {
      return false;
    } else {
      throw std::logic_error("Cannot have Write-After-Read dependencies in SSA form from " + clang_stmt_printer(op1) +  " to " + clang_stmt_printer(op2) + "\n");
    }
  }

  // op1 writes a variable (LHS) that op2 reads. (Read After Write)
  return (op_reads_var(op2, op1->getLHS()));
}

std::map<uint32_t, std::vector<InstBlock>> generate_partitions(const CompoundStmt * function_body) {
  // Verify that it's in SSA
  if (not is_in_ssa(function_body)) {
    throw std::logic_error("Partitioning will run only after program is in SSA form. This program isn't.");
  }
  // Append to a vector of const BinaryOperator *
  // in order of statement occurence.
  std::vector<const BinaryOperator *> stmt_vector;
  for (const auto * child : function_body->children()) {
    assert_exception(isa<BinaryOperator>(child));
    const auto * bin_op = dyn_cast<BinaryOperator>(child);
    assert_exception(bin_op->isAssignmentOp());
    stmt_vector.emplace_back(bin_op);
  }

  // Dependency graph creation
  Graph<const BinaryOperator *> dep_graph(clang_stmt_printer);
  for (const auto * stmt : stmt_vector) {
    dep_graph.add_node(stmt);
  }

  // Handle state variables specially
  dep_graph = handle_state_vars(stmt_vector, dep_graph);

  // Now add all Read After Write Dependencies, comparing a statement only with
  // a successor statement
  for (uint32_t i = 0; i < stmt_vector.size(); i++) {
    for (uint32_t j = i + 1; j < stmt_vector.size(); j++) {
      if (depends(stmt_vector.at(i), stmt_vector.at(j))) {
        dep_graph.add_edge(stmt_vector.at(i), stmt_vector.at(j));
      }
    }
  }

  // Eliminate nodes with no outgoing or incoming edge
  std::set<const BinaryOperator *> nodes_to_remove;
  for (const auto & node : dep_graph.node_set()) {
    if (dep_graph.pred_map().at(node).empty() and
        dep_graph.succ_map().at(node).empty()) {
      nodes_to_remove.emplace(node);
    }
  }
  for (const auto & node : nodes_to_remove) {
    dep_graph.remove_singleton_node(node);
  }

  std::cerr << dep_graph << std::endl;

  // Condense (https://en.wikipedia.org/wiki/Strongly_connected_component)
  // dep_graph after collapsing strongly connected components into one node
  // Pass a function to order statements within the sccs
  const auto & condensed_graph = dep_graph.condensation([] (const BinaryOperator * op1, const BinaryOperator * op2)
                                                        {return op1->getLocStart() < op2->getLocStart();});

  // Partition condensed graph using critical path scheduling
  const auto & partitioning = condensed_graph.critical_path_schedule();

  // Output partition into valid C code, one for each timestamp
  std::map<uint32_t, std::vector<InstBlock>> atom_bodies;
  std::vector<std::pair<InstBlock, uint32_t>> sorted_pairs(partitioning.begin(), partitioning.end());
  std::sort(sorted_pairs.begin(), sorted_pairs.end(), [] (const auto & x, const auto & y) { return x.second < y.second; });
  std::for_each(sorted_pairs.begin(), sorted_pairs.end(), [&atom_bodies] (const auto & pair)
                { if (atom_bodies.find(pair.second) == atom_bodies.end()) atom_bodies[pair.second] = std::vector<InstBlock>();
                  atom_bodies.at(pair.second).emplace_back(pair.first); });

  // Draw pipeline
  uint32_t max_stage_id = 0;
  uint32_t max_atom_id  = 0;
  PipelineDrawing atoms_for_drawing;
  for (const auto & body_pair : atom_bodies) {
    uint32_t atom_id = 0;
    const uint32_t stage_id = body_pair.first;
    for (const auto & atom_body : body_pair.second) {
      atoms_for_drawing[stage_id][atom_id] = atom_body;
      max_atom_id = std::max(max_atom_id, atom_id);
      atom_id++;
    }
    max_stage_id = std::max(max_stage_id, stage_id);
  }
  std::cerr << draw_pipeline(atoms_for_drawing, condensed_graph) << std::endl;
  std::cerr << "// " + std::to_string(max_stage_id + 1) + " stages" << std::endl;
  std::cerr << "// " + std::to_string(max_atom_id  + 1) + " atoms/stage" << std::endl;
  return atom_bodies;
}

std::string draw_pipeline(const PipelineDrawing & atoms_for_drawing, const Graph<InstBlock> & condensed_graph) {
  // Preamble for dot (node shape, fontsize etc)
  std::string ret = "digraph pipeline_diagram {splines=true node [shape = box style=\"rounded,filled\" fontsize = 10];\n";
  const uint32_t scale_x = 250;
  const uint32_t scale_y = 75;

  // Print out nodes
  for (const auto & stageid_with_atom_map : atoms_for_drawing) {
    const uint32_t stage_id = stageid_with_atom_map.first;
    for (const auto & atom_pair : stageid_with_atom_map.second) {
      const uint32_t atom_id = atom_pair.first;
      const auto atom = atoms_for_drawing.at(stage_id).at(atom_id);
      const auto atom_as_str = inst_block_printer(atom);
      ret += hash_string(atom_as_str) + " [label = \""
                                      + atom_as_str + "\""
                                      + "  pos = \""
                                      + std::to_string(scale_x * stage_id) + "," + std::to_string(scale_y * atom_id) + "\""
                                      + " fillcolor=" + (atom.size() > 1 ? "darkturquoise" : "white")
                                      + "];\n";
    }
  }

  // Print out edges
  for (const auto & node_pair : condensed_graph.succ_map())
    for (const auto & neighbor : node_pair.second)
      ret += hash_string(inst_block_printer(node_pair.first)) + " -> " +
             hash_string(inst_block_printer(neighbor)) + " ;\n";
  ret += "}";
  return ret;
}

std::string partitioning_transform(const TranslationUnitDecl * tu_decl) {
  const auto & id_set = identifier_census(tu_decl);

  // Storage for returned string
  std::string ret;

  // Create unique identifier generator
  UniqueIdentifiers unique_identifiers(id_set);

  for (const auto * child_decl : dyn_cast<DeclContext>(tu_decl)->decls()) {
    assert_exception(child_decl);
    if (isa<VarDecl>(child_decl) or
        isa<RecordDecl>(child_decl)) {
      // Pass through these declarations as is
      ret += clang_decl_printer(child_decl) + ";";
    } else if (isa<FunctionDecl>(child_decl) and (not is_packet_func(dyn_cast<FunctionDecl>(child_decl)))) {
      ret += generate_scalar_func_def(dyn_cast<FunctionDecl>(child_decl));
    } else if (isa<FunctionDecl>(child_decl) and (is_packet_func(dyn_cast<FunctionDecl>(child_decl)))) {
      const auto * function_decl = dyn_cast<FunctionDecl>(child_decl);

      // Extract function signature
      assert_exception(function_decl->getNumParams() >= 1);
      const auto * pkt_param = function_decl->getParamDecl(0);
      const auto pkt_type  = function_decl->getParamDecl(0)->getType().getAsString();
      const auto pkt_name = clang_value_decl_printer(pkt_param);

      // Transform function body
      const auto atom_bodies = generate_partitions(dyn_cast<CompoundStmt>(function_decl->getBody()));

      // Create atom functions with new bodies, encode stage_id and atom_id within function body
      // Also store these atom function bodies as strings for the call to draw_pipeline below
      for (const auto & body_pair : atom_bodies) {
        uint32_t atom_id = 0;
        const uint32_t stage_id = body_pair.first;
        for (const auto & atom_body : body_pair.second) {
          const auto atom_body_as_str = function_decl->getReturnType().getAsString() + " " +
                                        "_atom_" + std::to_string(stage_id) + "_" + std::to_string(atom_id) +
                                        "( " + pkt_type + " " +  pkt_name + ") { " +
                                        inst_block_printer(atom_body) + "}\n";
          atom_id++;
          ret += atom_body_as_str;
        }
      }
    }
  }
  return ret;
}
