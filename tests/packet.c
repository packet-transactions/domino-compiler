struct Packet {
  int x;
  int y;
};

void func(struct Packet p) {
  p.x = 1;
  p.y = 2;
}
