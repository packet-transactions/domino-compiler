#define THRESHOLD 5 // Flowlet gap: TODO, should be something configurable

typedef struct Packet {
  int new_next_hop;
  int time;
  int next_hop;
} Packet;

int last_time;
int next_hop;

void func(Packet pkt) {
  pkt.new_next_hop = 2; // TODO: 2 should technically be rand()
  if (pkt.time - last_time > THRESHOLD) {
    next_hop = pkt.new_next_hop;
  }
  last_time = pkt.time;
  pkt.next_hop = next_hop;
}
