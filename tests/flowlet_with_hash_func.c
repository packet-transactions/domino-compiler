#define NUM_HOPS 10
#define THRESHOLD 5

struct Packet {
  int sport;
  int dport;
  int new_hop;
  int arrival_time;
  int next_hop;
};

int hash(int, int);

int last_time = 0;
int saved_hop = 0;

void func(struct Packet pkt) {
  pkt.new_hop = hash(pkt.sport, pkt.dport);
  if (pkt.arrival_time - last_time > THRESHOLD) {
    saved_hop = pkt.new_hop;
  }
  last_time = pkt.arrival_time;
  pkt.next_hop = saved_hop;
}
