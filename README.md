[![Build Status](https://travis-ci.org/anirudhSK/domino.svg?branch=master)](https://travis-ci.org/anirudhSK/domino)

1. Ensure you have python-3.5.1 (https://www.python.org/ftp/python/3.5.1/python-3.5.1-macosx10.6.pkg) git g++ build-essential autotools and zlib1g-dev (either using macports or apt-get)
2. Get clang + llvm from http://llvm.org/releases/3.5.0/clang+llvm-3.5.0-macosx-apple-darwin.tar.xz (or the equivalent files for Linux)
3. Get sketch from https://people.csail.mit.edu/asolar/sketch-1.6.9.tar.gz; add the sketch binary to the path
4. git clone https://github.com/anirudhSK/banzai.git; cd banzai; ./autogen.sh; ./configure CXX='g++-4.9'; sudo make install; cd ..; ./autogen.sh && ./configure CXX='g++-4.9' && make
5. git clone https://github.com/anirudhsk/domino
6. ./autogen.sh
7. ./configure CLANG_DEV_LIBS=<wherever you downloaded clang-3.5>
8. make
9. make check
