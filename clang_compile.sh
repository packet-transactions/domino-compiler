g++ --std=c++14 -isystem /usr/lib/llvm-3.5/include/ \
	-pedantic -Wconversion -Wsign-conversion -Wall -Wextra -Weffc++ -Werror -fno-default-inline \
	$@ \
  libclangall.a \
	`llvm-config --ldflags --libs --system-libs`
