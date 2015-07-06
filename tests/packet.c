typedef struct Packet {
  int x;
  int y;
} Packet;

void func(Packet p) {
  p.x = 1;
  p.y = 2;
}
