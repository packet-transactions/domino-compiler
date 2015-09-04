#include "hashes.h"

#define low_th 100
#define hi_th  1000

struct Packet {
  int sport;
  int dport;
  int is_not_heavy_hitter;
};

int sketch_cnt_1[4096] = {0};
int sketch_cnt_2[4096] = {0};
int sketch_cnt_3[4096] = {0};

void func(struct Packet p) {
  if (sketch_cnt_1[hash(p.sport, p.dport) % 4096] > low_th && sketch_cnt_1[hash(p.sport, p.dport) % 4096]< hi_th &&
	    sketch_cnt_2[hash(p.sport, p.dport) % 4096] > low_th && sketch_cnt_2[hash(p.sport, p.dport) % 4096] < hi_th &&
	    sketch_cnt_3[hash(p.sport, p.dport) % 4096] > low_th && sketch_cnt_3[hash(p.sport, p.dport) % 4096] < hi_th) {
		p.is_not_heavy_hitter = 0;
  }	else {
		p.is_not_heavy_hitter = 1;
  }
	sketch_cnt_1[hash(p.sport, p.dport) % 4096] = sketch_cnt_1[hash(p.sport, p.dport) % 4096] + 1;
	sketch_cnt_2[hash(p.sport, p.dport) % 4096] = sketch_cnt_2[hash(p.sport, p.dport) % 4096] + 1;
	sketch_cnt_3[hash(p.sport, p.dport) % 4096] = sketch_cnt_3[hash(p.sport, p.dport) % 4096] + 1;
}
