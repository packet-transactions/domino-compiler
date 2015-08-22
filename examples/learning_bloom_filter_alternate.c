struct Packet {
  int sport;
  int dport;
  int member;
};

int filter1[256];
int filter2[256];
int filter3[256];

int hash1(int a, int b) {
  return a * b;
}

int hash2(int a, int b) {
  return a * b;
}

int hash3(int a, int b) {
  return a * b;
}

void func(struct Packet pkt) {
  if (filter1[hash1(pkt.sport, pkt.dport)] &&
      filter2[hash2(pkt.sport, pkt.dport)] &&
      filter3[hash3(pkt.sport, pkt.dport)]) {
    pkt.member = 1;
  } else {
    filter1[hash1(pkt.sport, pkt.dport)] = 1;
    filter2[hash2(pkt.sport, pkt.dport)] = 1;
    filter3[hash3(pkt.sport, pkt.dport)] = 1;
  }
}
