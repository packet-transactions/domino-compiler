struct Packet {
  int sport;
  int dport;
  int member;
};

int filter1[256] = {1};
int filter2[256] = {1};
int filter3[256] = {1};

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
  pkt.member = (filter1[hash1(pkt.sport, pkt.dport)] &&
                filter2[hash2(pkt.sport, pkt.dport)] &&
                filter3[hash3(pkt.sport, pkt.dport)]);

  filter1[hash1(pkt.sport, pkt.dport)] = 1;
  filter2[hash2(pkt.sport, pkt.dport)] = 1;
  filter3[hash3(pkt.sport, pkt.dport)] = 1;
}
