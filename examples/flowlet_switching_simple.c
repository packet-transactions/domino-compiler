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
  int last_time_idx;
  int saved_hop_idx;
};

int last_time [NUM_FLOWLETS] = {0};
int saved_hop [NUM_FLOWLETS] = {0};

void flowlet(struct Packet pkt) {
  pkt.new_hop   = hash3(pkt.sport, pkt.dport, pkt.arrival_time) % NUM_HOPS;
  pkt.last_time_idx = hash2(pkt.sport, pkt.dport) % NUM_FLOWLETS;
  pkt.saved_hop_idx = hash2(pkt.sport, pkt.dport) % NUM_FLOWLETS;
  if (pkt.arrival_time - last_time[pkt.last_time_idx] >
      FLOWLET_THRESHOLD) {
    saved_hop[pkt.saved_hop_idx] = pkt.new_hop;
  }
  last_time[pkt.last_time_idx] = pkt.arrival_time;
  pkt.next_hop = saved_hop[pkt.saved_hop_idx];
}
