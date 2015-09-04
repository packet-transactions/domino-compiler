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

void func(struct Packet pkt) {
  if (pkt.bloom_op) {
    pkt.member = (filter1[hash2(pkt.sport, pkt.dport) % 256] &&
                  filter2[hash2(pkt.sport, pkt.dport) % 256] &&
                  filter3[hash2(pkt.sport, pkt.dport) % 256]);
  } else {
    filter1[hash2(pkt.sport, pkt.dport) % 256] = 1;
    filter2[hash2(pkt.sport, pkt.dport) % 256] = 1;
    filter3[hash2(pkt.sport, pkt.dport) % 256] = 1;
  }
}
