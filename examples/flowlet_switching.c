#include "hashes.h"
#define NUM_FLOWLETS      8000
#define FLOWLET_THRESHOLD 5

struct Packet {
  int src_port;
  int dst_port;
  int src_addr;
  int dst_addr;
  int protocol;
  int new_hop;
  int arrival_time;
  int next_hop;
  int last_time_idx;
  int saved_hop_idx;
};

int last_time [NUM_FLOWLETS] = {0};
int saved_hop [NUM_FLOWLETS] = {0};

void flowlet(struct Packet pkt) {
  pkt.new_hop   = hash6(pkt.src_port, pkt.dst_port,
                        pkt.src_addr, pkt.dst_addr,
                        pkt.protocol, pkt.arrival_time);
  pkt.last_time_idx = hash5(pkt.src_port, pkt.dst_port,
                            pkt.src_addr, pkt.dst_addr,
                            pkt.protocol) % NUM_FLOWLETS;
  pkt.saved_hop_idx = hash5(pkt.src_port, pkt.dst_port,
                            pkt.src_addr, pkt.dst_addr,
                            pkt.protocol) % NUM_FLOWLETS;
  if (pkt.arrival_time - last_time[pkt.last_time_idx] >
      FLOWLET_THRESHOLD) {
    saved_hop[pkt.saved_hop_idx] = pkt.new_hop;
  }
  last_time[pkt.last_time_idx] = pkt.arrival_time;
  pkt.next_hop = saved_hop[pkt.saved_hop_idx];
}
