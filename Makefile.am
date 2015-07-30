AM_CXXFLAGS = $(PICKY_CXXFLAGS)
AM_CPPFLAGS = -isystem /usr/lib/llvm-3.5/include/
AM_LDFLAGS = $(LLVM_LDFLAGS)
LDADD = libclangall.a $(LLVM_LIBS) $(LLVM_SYSLIBS)
common_source = clang_utility_functions.cc  expr_functions.cc clang_utility_functions.h expr_functions.h state_var_decl_handler.h state_var_decl_handler.cc packet_decl_creator.h packet_decl_creator.cc run_pass.h func_transform_handler.h func_transform_handler.cc packet_variable_census.cc packet_variable_census.h unique_var_generator.h unique_var_generator.cc

bin_PROGRAMS = partitioning_prog if_conversion_prog expr_flattener_prog strength_reductor

partitioning_prog_SOURCES = $(common_source) partitioning_handler.cc  partitioning_prog.cc partitioning_handler.h
if_conversion_prog_SOURCES = $(common_source) if_conversion_prog.cc if_conversion_handler.cc if_conversion_handler.h 
expr_flattener_prog_SOURCES = $(common_source) expr_flattener_prog.cc expr_flattener_handler.cc expr_flattener_handler.h
strength_reductor_SOURCES = $(common_source) strength_reduction.cc strength_reduction_handler.h strength_reduction_handler.cc
