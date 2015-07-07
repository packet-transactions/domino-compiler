void stage1() {
  if (x) {
    x = x + 1;
  }
}

void stage2(Packet p) {
  p.a = x;
  p.b = x + 1;
}
