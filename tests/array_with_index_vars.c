int a[256];
struct Packet {int x;};

void func(struct Packet p) {
  a[p.x] = 1;
  a[p.x] = a[p.x] + 1;
}
