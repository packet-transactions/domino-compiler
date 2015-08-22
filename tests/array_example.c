int a[256];

struct Packet {
  int x;
};

void func(struct Packet p) {
  a[0] = a[0] + 1;
}
