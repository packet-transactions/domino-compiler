struct Packet {
  int a;
};

void foo(struct Packet pkt) {
  if (1) {
    pkt.a = 0;
  }
}
