struct Packet {
  int x;
};

void foo(struct Packet pkt) {
  if(1) pkt.x = 1;
}
