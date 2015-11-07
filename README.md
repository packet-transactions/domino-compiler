[![Build Status](https://magnum.travis-ci.com/anirudhSK/domino.svg?token=E3TBAuEumusxDa6hAgxA)](https://magnum.travis-ci.com/anirudhSK/domino)

0. sudo add-apt-repository -y ppa:ubuntu-toolchain-r/test
1. sudo apt-get update -qq
2. sudo apt-get install -qq g++-4.9
3. sudo apt-get install git clang-format-3.5 git build-essential autoconf libtool zlib1g-dev
4. curl -sSL http://llvm.org/apt/llvm-snapshot.gpg.key | sudo -E apt-key add -
5. sudo -E add-apt-repository -y 'deb http://llvm.org/apt/precise/ llvm-toolchain-precise-3.5 main'
6. sudo apt-get install -y -qq libclang-3.5-dev
7. sudo apt-get install -y -qq llvm-3.5-dev
8. git clone https://github.com/anirudhSK/banzai.git; cd banzai; ./autogen.sh; ./configure CXX='g++-4.9'; sudo make install; cd ..; ./autogen.sh && ./configure CXX='g++-4.9' && make
9. git clone https://github.com/anirudhsk/domino
10. ./autogen.sh
11. CXX='g++-4.9' ./configure
12. make
13. make check
