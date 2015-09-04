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
  int flow_hash;
};

int last_time [NUM_FLOWLETS] = {0};
int saved_hop [NUM_FLOWLETS] = {0};

void func(struct Packet pkt) {
  pkt.flow_hash = hash5(pkt.src_port, pkt.dst_port,
                        pkt.src_addr, pkt.dst_addr,
                        pkt.protocol);
  pkt.new_hop   = hash6(pkt.src_port, pkt.dst_port,
                        pkt.src_addr, pkt.dst_addr,
                        pkt.protocol, pkt.arrival_time);
  if (pkt.arrival_time - last_time[pkt.flow_hash % NUM_FLOWLETS] > FLOWLET_THRESHOLD) {
    saved_hop[pkt.flow_hash % NUM_FLOWLETS] = pkt.new_hop;
  }
  last_time[pkt.flow_hash % NUM_FLOWLETS] = pkt.arrival_time;
  pkt.next_hop = saved_hop[pkt.flow_hash % NUM_FLOWLETS];
}
