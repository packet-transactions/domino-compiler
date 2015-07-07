int x;

typedef struct Packet {
  int a;
  int b;
  int c;
} Packet;

void foo(Packet p) {
  int tmp__5467;
  int p__a;
  int p__b;

  tmp__5467 = x;
  x = (tmp__5467 ? (x + 1) : x);

  p__a = x;
  p__b = x + 1;
}
