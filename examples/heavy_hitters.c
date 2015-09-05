#include "hashes.h"

#define low_th 100
#define hi_th  1000

struct Packet {
  int sport;
  int dport;
  int is_not_heavy_hitter;
  int sketch1_idx;
  int sketch2_idx;
  int sketch3_idx;
};

#define NUM_ENTRIES 4096

int sketch_cnt_1[NUM_ENTRIES] = {0};
int sketch_cnt_2[NUM_ENTRIES] = {0};
int sketch_cnt_3[NUM_ENTRIES] = {0};

void func(struct Packet p) {
  p.sketch1_idx = hash2(p.sport, p.dport) % NUM_ENTRIES;
  p.sketch2_idx = hash2(p.sport, p.dport) % NUM_ENTRIES;
  p.sketch3_idx = hash2(p.sport, p.dport) % NUM_ENTRIES;
  if (sketch_cnt_1[p.sketch1_idx] > low_th && sketch_cnt_1[p.sketch1_idx] < hi_th &&
	    sketch_cnt_2[p.sketch2_idx] > low_th && sketch_cnt_2[p.sketch2_idx] < hi_th &&
	    sketch_cnt_3[p.sketch3_idx] > low_th && sketch_cnt_3[p.sketch3_idx] < hi_th) {
		p.is_not_heavy_hitter = 0;
  }	else {
		p.is_not_heavy_hitter = 1;
  }
	sketch_cnt_1[p.sketch1_idx]+= 1;
	sketch_cnt_2[p.sketch2_idx]+= 1;
	sketch_cnt_3[p.sketch3_idx]+= 1;
}
