AM_CXXFLAGS = $(PICKY_CXXFLAGS)
AM_CPPFLAGS = -isystem /usr/lib/llvm-3.5/include/
AM_LDFLAGS = $(LLVM_LDFLAGS)
LDADD = libclangall.a -lLLVMOption  -lLLVMMCParser -lLLVMBitReader  -lLLVMMC -lLLVMSupport -lz -lpthread -ltinfo -ldl

common_source = clang_utility_functions.cc  expr_functions.cc clang_utility_functions.h expr_functions.h packet_variable_census.cc packet_variable_census.h unique_var_generator.h unique_var_generator.cc single_pass.h pkt_func_transform.h pkt_func_transform.cc graph.h cstr_array.h util.h file_descriptor.hh file_descriptor.cc temp_file.hh temp_file.cc exception.hh prog_transforms.h strength_reducer.cc expr_flattener_handler.cc expr_flattener_handler.h expr_propagater.cc stateful_flanks.cc ssa.cc dep_graph.cc

bin_PROGRAMS = if_converter echo rewritersample

if_converter_SOURCES = $(common_source) if_converter.cc if_conversion_handler.cc if_conversion_handler.h
echo_SOURCES = $(common_source) echo.cc
rewritersample_SOURCES = $(common_source) rewritersample.cc
