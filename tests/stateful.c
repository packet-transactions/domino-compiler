int x;

typedef struct Packet {
  int a;
  int b;
  int c;
} Packet;

void foo(Packet p) {
  if (x) {
    x = x + 1;
  }

  p.a = x;
  p.b = x + 1;
}
