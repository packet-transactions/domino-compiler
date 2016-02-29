#! /usr/bin/python
from ast import NodeVisitor
from ast import parse
from ast import dump
from ast import Name
from ast import Subscript
from ast import FunctionDef
from ast import Assign
from ast import Num
from sys import argv
from io  import StringIO
from unparse import Unparser

# Check if target is a state variable (scalar/Name or array/Subscript)
def track_state_vars(target):
  if type(target) is Name:
    return target.id
  elif type(target) is Subscript:
    return target.value.id
  else:
    return None

# Track reads and writes to state (global) variables
class StateReadWrite(NodeVisitor):
  def __init__(self):
    self.write_set = set()
    self.read_set = set()
    self.write_set.add(None)
    self.read_set.add(None)

  # Writes
  def visit_Assign(self, node):
    for target in node.targets:
      self.write_set.add(track_state_vars(target))
  def visit_AugAssign(self, node):
    for target in node.targets:
      self.write_set.add(track_state_vars(target))

  # Reads
  def visit_Name(self, node):
    self.read_set.add(node.id)
  def visit_Subscript(self, node):
    self.read_set.add(node.value.id) 

class FieldCollector(NodeVisitor):
  def __init__(self):
    self.field_set = set()
  def visit_Attribute(self, node):
    self.field_set.add(node.attr)

class PyToC(NodeVisitor):
  def __init__(self):
    self.read_set = set()
    self.write_set = set()
    self.field_set = set()
    self.program_output = StringIO()

  def visit_FunctionDef(self, node):
    # Track reads and writes to state (global) variables
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
assert(len(function_def_nodes) == 1)

# Parse the function alone and generate all the data structures we need
pytoc_converter = PyToC()
pytoc_converter.visit(function_def_nodes[0])

# Now print out the converted output
# Print out state
for state in state_assignments:
  # Constant state
  if (state in pytoc_converter.read_set and state not in pytoc_converter.write_set):
    assert(type(state_assignments[state]) is Num)
    print("#define ", state, state_assignments[state].n);
  # Mutable state
  elif (state in pytoc_converter.write_set):
    assert(type(state_assignments[state]) is Num)
    print("int ", state, " = ", state_assignments[state].n, ";");

# Print out fields
print("struct Packet {");
for field in pytoc_converter.field_set:
  print("int ", field, ";")
print("};")

# Print out program
print(pytoc_converter.program_output.getvalue())
#print(dump(parse(open(sys.argv[1]).read())))
