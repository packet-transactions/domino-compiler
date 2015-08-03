AM_CXXFLAGS = $(PICKY_CXXFLAGS)
AM_CPPFLAGS = -isystem /usr/lib/llvm-3.5/include/
AM_LDFLAGS = $(LLVM_LDFLAGS)
LDADD = libclangall.a $(LLVM_LIBS) $(LLVM_SYSLIBS)
common_source = clang_utility_functions.cc  expr_functions.cc clang_utility_functions.h expr_functions.h run_pass.h packet_variable_census.cc packet_variable_census.h unique_var_generator.h unique_var_generator.cc single_pass.cc single_pass.h echo_handler.h pkt_func_transform.h pkt_func_transform.cc

bin_PROGRAMS = partitioner if_converter expr_flattener strength_reducer expr_propagater echo

partitioner_SOURCES = $(common_source) partitioning_handler.cc  partitioner.cc partitioning_handler.h
if_converter_SOURCES = $(common_source) if_converter.cc if_conversion_handler.cc if_conversion_handler.h
expr_flattener_SOURCES = $(common_source) expr_flattener.cc expr_flattener_handler.cc expr_flattener_handler.h
strength_reducer_SOURCES = $(common_source) strength_reducer.cc
expr_propagater_SOURCES = $(common_source) expr_propagater.cc
echo_SOURCES = $(common_source) echo.cc
