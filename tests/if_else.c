struct Packet {
 int a;
};

void foo(struct Packet pkt) {
  pkt.a = 0;
  if (1) {
    pkt.a = 0;
  } else {
    pkt.a = 1;
  }
}
