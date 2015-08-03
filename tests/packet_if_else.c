struct Packet {
  int x;
  int y;
  int z;
  int w;
};

void func(struct Packet p) {
  p.x = 1;
  p.y = 2;
  if (p.x == 3) {
    if (p.y == 5) {
      p.z = 3;
    } else  {
      p.w = 2;
    }
  } else {
    p.x = p.y + 23;
    p.w = p.x;
  }
}
