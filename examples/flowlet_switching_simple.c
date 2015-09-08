#include "hashes.h"

#define NUM_FLOWLETS      8000
#define FLOWLET_THRESHOLD 5
#define NUM_HOPS          10

struct Packet {
  int sport;
  int dport;
  int new_hop;
  int arrival;
  int next_hop;
  int idl; // index into last_time
  int ids; // index into saved_hop
};

int last_time [NUM_FLOWLETS] = {0};
int saved_hop [NUM_FLOWLETS] = {0};

void flowlet(struct Packet pkt) {
  pkt.new_hop = hash3(pkt.sport,
                      pkt.dport,
                      pkt.arrival);
                % NUM_HOPS;

  pkt.idl = hash2(pkt.sport,
                  pkt.dport)
            % NUM_FLOWLETS;

  pkt.ids = hash2(pkt.sport,
                  pkt.dport)
            % NUM_FLOWLETS;

  if (pkt.arrival_time -
      last_time[pkt.idl] >
      FLOWLET_THRESHOLD) {
    saved_hop[pkt.ids] = pkt.new_hop;
  }

  last_time[pkt.idl] = pkt.arrival;
  pkt.next_hop = saved_hop[pkt.ids];
}
