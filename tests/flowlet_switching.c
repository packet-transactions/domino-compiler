#define NUM_HOPS 10
#define THRESHOLD 5 // Flowlet gap: TODO, should be something configurable

typedef struct Packet {
  int sport;
  int dport;
  int new_hop;
  int arrival_time;
  int next_hop;
} Packet;

int last_time;
int next_hop;

void func(Packet pkt) {
  pkt.new_hop = (pkt.sport * pkt.dport) % NUM_HOPS;
  if (pkt.arrival_time - last_time > THRESHOLD) {
    next_hop = pkt.new_hop;
  }
  last_time = pkt.arrival_time;
  pkt.next_hop = next_hop;
}
