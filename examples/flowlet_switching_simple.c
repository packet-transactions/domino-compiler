#include "hashes.h"
#define NUM_FLOWLETS      8000
#define FLOWLET_THRESHOLD 5
#define NUM_HOPS 64

struct Packet {
  int sport;
  int dport;
  int new_hop;
  int arrival_time;
  int next_hop;
  int id0;
  int id1;
};

int last_time [NUM_FLOWLETS] = {0};
int saved_hop [NUM_FLOWLETS] = {0};

void flowlet(struct Packet pkt) {
  pkt.new_hop   = hash3(pkt.sport, pkt.dport, pkt.arrival_time) % NUM_HOPS;
  pkt.id1 = hash2(pkt.sport, pkt.dport) % NUM_FLOWLETS;
  pkt.id0 = hash2(pkt.sport, pkt.dport) % NUM_FLOWLETS;
  if (pkt.arrival_time - last_time[pkt.id1] >
      FLOWLET_THRESHOLD) {
    saved_hop[pkt.id0] = pkt.new_hop;
  }
  last_time[pkt.id1] = pkt.arrival_time;
  pkt.next_hop = saved_hop[pkt.id0];
}
