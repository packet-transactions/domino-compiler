struct Packet {
  int a;
};

void foo(struct Packet pkt) {
  if (1) {
    pkt.a = 1;
    if (2) {
      pkt.a = 2;
    }
  }
}
