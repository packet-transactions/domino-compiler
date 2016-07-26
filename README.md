[![Build Status](https://travis-ci.org/anirudhSK/domino.svg?branch=master)](https://travis-ci.org/anirudhSK/domino)

1. Ensure you have python-3.5.1 git g++ build-essential autotools and zlib1g-dev (either using macports or apt-get)
2. Get clang + llvm from http://llvm.org/releases/3.5.0/clang+llvm-3.5.0-macosx-apple-darwin.tar.xz (or the equivalent files for Linux)
3. git clone https://github.com/anirudhSK/banzai.git; cd banzai; ./autogen.sh; ./configure CXX='g++-4.9'; sudo make install; cd ..; ./autogen.sh && ./configure CXX='g++-4.9' && make
4. git clone https://github.com/anirudhsk/domino
5. ./autogen.sh
6. ./configure CLANG_DEV_LIBS=<wherever you downloaded clang-3.5>
7. make
8. make check
