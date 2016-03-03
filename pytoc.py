#! /usr/bin/python
from ast     import NodeVisitor
from ast     import parse
from ast     import dump
from ast     import Name
from ast     import Subscript
from ast     import FunctionDef
from ast     import Assign
from ast     import BinOp
from ast     import Num
from sys     import argv
from io      import StringIO
from unparse import Unparser

# Extract name of state variable from either
# 1. scalar state variable (Name) or
# 2. state variable array (Subscript)
def track_state_vars(target):
  if type(target) is Name:
    return target.id
  elif type(target) is Subscript:
    return target.value.id
  else:
    return None

# Track reads and writes to state variables in function body
class StateReadWrite(NodeVisitor):
  def __init__(self):
    self.write_set = set()
    self.read_set = set()
    self.write_set.add(None)
    self.read_set.add(None)

  # Writes (Assign and AugAssign)
  def visit_Assign(self, node):
    for target in node.targets:
      self.write_set.add(track_state_vars(target))
    self.generic_visit(node)
  def visit_AugAssign(self, node):
    for target in node.targets:
      self.write_set.add(track_state_vars(target))
    self.generic_visit(node)

  # Reads (scalars and arrays)
  def visit_Name(self, node):
    self.read_set.add(node.id)
    self.generic_visit(node)

  def visit_Subscript(self, node):
    self.read_set.add(node.value.id) 
    self.generic_visit(node)

# Collect fields used in function body
class FieldCollector(NodeVisitor):
  def __init__(self):
    self.field_set = set()
  def visit_Attribute(self, node):
    self.field_set.add(node.attr)

# The core logic,
# go through the function body,
# identify state variables and packet fields
class PyToC(NodeVisitor):
  def __init__(self):
    self.read_set = set()
    self.write_set = set()
    self.field_set = set()
    self.program_output = StringIO()

  def visit_FunctionDef(self, node):
    # Compute read and write sets for state variables
    state_read_write = StateReadWrite()
    state_read_write.visit(node)
    self.read_set = state_read_write.read_set
    self.read_set.remove(None)
    self.write_set = state_read_write.write_set
    self.write_set.remove(None)

    # Collect fields
    field_collector = FieldCollector()
    field_collector.visit(node)
    self.field_set = field_collector.field_set

    # Write out function body
    print("void func(struct Packet p) {", file = self.program_output)
    for stmt in node.body:
      self.visit(stmt)
    print("}", file = self.program_output)

  # Visitors for all other nodes traversed recursively from FunctionDef
  # Use Unparser to turn nodes into text
  def visit_Compare(self, node):
    Unparser(node, self.program_output)

  def visit_Assign(self, node):
    Unparser(node, self.program_output)
    print(";\n", file = self.program_output)

  def visit_If(self, node):
    print("if ", file = self.program_output)
    self.visit(node.test)
    print(" {", file = self.program_output)
    for stmt in node.body:
      self.visit(stmt)
    print("} else {", file = self.program_output)
    for stmt in node.orelse:
      self.visit(stmt)
    print("}", file = self.program_output)

# Main function in the program
# Parse the top level and generate nodes
# corresponding to
# 1. state assignments, and
# 2. function definitions.
top_level = parse(open(argv[1]).read())
function_def_nodes = []
state_assignments = dict()
for node in top_level.body:
  if type(node) is FunctionDef:
    function_def_nodes += [node]
  elif type(node) is Assign:
    for target in node.targets:
      state_assignments[target.id] = node.value
  else:
    assert(False)
# There can be only a single function definition
assert(len(function_def_nodes) == 1)

# Parse the function alone and generate all the data structures we need
pytoc_converter = PyToC()
pytoc_converter.visit(function_def_nodes[0])

# Include the hashes header
print("#include \"headers.h\"")

# Now print out the converted output
# Print out state, constant state first, then mutable state
constant_state = ""
mutable_state  = ""
for state in state_assignments:
  # Convert constant state into C macros
  if (state in pytoc_converter.read_set and state not in pytoc_converter.write_set):
    assert(type(state_assignments[state]) is Num)
    constant_state += "#define " + str(state) + " " + str(state_assignments[state].n) + "\n"
  # Convert mutable state into C arrays or scalars
  elif (state in pytoc_converter.write_set):
    if (type(state_assignments[state]) is Num):
      mutable_state += "int " + str(state) + " = " + str(state_assignments[state].n) + ";\n"
    elif (type(state_assignments[state]) is BinOp):
      # This is an array initializer in python
      element_value = state_assignments[state].left.elts[0].n;
      if (type(state_assignments[state].right) is Num):
        num_elements  = state_assignments[state].right.n
      elif (type(state_assignments[state].right) is Name):
        num_elements  = state_assignments[state].right.id
      else:
        assert(False)
      mutable_state += "int " + str(state) + " [" + str(num_elements) + "] = {" + str(element_value) + "};\n"
    else:
      assert(False)
  else:
    assert(False)
print(constant_state + mutable_state)

# Print out fields
print("struct Packet {");
for field in pytoc_converter.field_set:
  print("int ", field, ";")
print("};")

# Print out program
print(pytoc_converter.program_output.getvalue())
#print(dump(parse(open(argv[1]).read())))
