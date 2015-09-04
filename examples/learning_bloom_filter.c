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
  pkt.member = (filter1[hash2(pkt.sport, pkt.dport) % 256] &&
                filter2[hash2(pkt.sport, pkt.dport) % 256] &&
                filter3[hash2(pkt.sport, pkt.dport) % 256]);

  filter1[hash2(pkt.sport, pkt.dport) % 256] = 1;
  filter2[hash2(pkt.sport, pkt.dport) % 256] = 1;
  filter3[hash2(pkt.sport, pkt.dport) % 256] = 1;
}
