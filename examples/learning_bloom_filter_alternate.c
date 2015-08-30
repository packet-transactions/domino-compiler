#include "hashes.h"

struct Packet {
  int sport;
  int dport;
  int member;
};

int filter1[256] = {0};
int filter2[256] = {0};
int filter3[256] = {0};

void func(struct Packet pkt) {
  if (filter1[hash(pkt.sport, pkt.dport) % 256] &&
      filter2[hash(pkt.sport, pkt.dport) % 256] &&
      filter3[hash(pkt.sport, pkt.dport) % 256]) {
    pkt.member = 1;
  } else {
    pkt.member = 0;
    filter1[hash(pkt.sport, pkt.dport) % 256] = 1;
    filter2[hash(pkt.sport, pkt.dport) % 256] = 1;
    filter3[hash(pkt.sport, pkt.dport) % 256] = 1;
  }
}
