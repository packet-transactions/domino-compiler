#include "hashes.h"

struct Packet {
  int sport;
  int dport;
  int member;
};

int filter1[256] = {1};
int filter2[256] = {1};
int filter3[256] = {1};

void func(struct Packet pkt) {
  pkt.member = (filter1[hash(pkt.sport, pkt.dport)] &&
                filter2[hash(pkt.sport, pkt.dport)] &&
                filter3[hash(pkt.sport, pkt.dport)]);

  filter1[hash(pkt.sport, pkt.dport)] = 1;
  filter2[hash(pkt.sport, pkt.dport)] = 1;
  filter3[hash(pkt.sport, pkt.dport)] = 1;
}
