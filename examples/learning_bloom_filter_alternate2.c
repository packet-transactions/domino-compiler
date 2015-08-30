#include "hashes.h"

struct Packet {
  int sport;
  int dport;
  int member;
};

int filter1[256];
int filter2[256];
int filter3[256];

void func(struct Packet pkt) {
  if (filter1[hash(pkt.sport, pkt.dport)] &&
      filter2[hash(pkt.sport, pkt.dport)] &&
      filter3[hash(pkt.sport, pkt.dport)]) {
    pkt.member = 1;
  }
  filter1[hash(pkt.sport, pkt.dport)] = 1;
  filter2[hash(pkt.sport, pkt.dport)] = 1;
  filter3[hash(pkt.sport, pkt.dport)] = 1;
}
