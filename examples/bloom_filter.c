#include "hashes.h" // For all the hash functions we need

struct Packet {
  int sport;
  int dport;
  int member;
  int bloom_op; // bloom_op = 1 is test, bloom_op = 0 is add
};

int filter1[256] = {0};
int filter2[256] = {0};
int filter3[256] = {0};

int hash(int a, int b) {
  int ret = (a * b) % 256;
  if (ret < 0) ret = 0;
  return ret;
}

void func(struct Packet pkt) {
  if (pkt.bloom_op) {
    pkt.member = (filter1[hash(pkt.sport, pkt.dport)] &&
                  filter2[hash(pkt.sport, pkt.dport)] &&
                  filter3[hash(pkt.sport, pkt.dport)]);
  } else {
    filter1[hash(pkt.sport, pkt.dport)] = 1;
    filter2[hash(pkt.sport, pkt.dport)] = 1;
    filter3[hash(pkt.sport, pkt.dport)] = 1;
  }
}
