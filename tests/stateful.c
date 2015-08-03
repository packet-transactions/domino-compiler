int x;

struct Packet {
  int a;
  int b;
  int c;
};

void foo(struct Packet p) {
  if (x) {
    x = x + 1;
  }

  p.a = x;
  p.b = x + 1;
}
