g++ --std=c++14 -isystem /usr/lib/llvm-3.5/include/ \
	-pedantic -Wconversion -Wsign-conversion -Wall -Wextra -Weffc++ -Werror -fno-default-inline \
	$@ \
	-L /usr/lib/llvm-3.5/lib/  \
	-Wl,--start-group \
	-lclangAST \
	-lclangAnalysis \
	-lclangBasic \
	-lclangDriver \
	-lclangEdit \
	-lclangFrontend \
	-lclangFrontendTool \
	-lclangLex \
	-lclangParse \
	-lclangSema \
	-lclangEdit \
	-lclangASTMatchers \
	-lclangRewrite \
	-lclangRewriteFrontend \
	-lclangSerialization \
	-lclangTooling \
	-Wl,--end-group \
	`llvm-config --ldflags --libs --system-libs`
