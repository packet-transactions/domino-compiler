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

int hash1(int a, int b) {
  return a * b;
}

int hash2(int a, int b) {
  return a * b;
}

int hash3(int a, int b) {
  return a * b;
}

void func(struct Packet p) {
  if (sketch_cnt_1[hash1(p.sport, p.dport)] > low_th && sketch_cnt_1[hash1(p.sport, p.dport)]< hi_th &&
	    sketch_cnt_2[hash2(p.sport, p.dport)] > low_th && sketch_cnt_2[hash2(p.sport, p.dport)] < hi_th &&
	    sketch_cnt_3[hash3(p.sport, p.dport)] > low_th && sketch_cnt_3[hash3(p.sport, p.dport)] < hi_th) {
		p.is_not_heavy_hitter = 0;
  }	else {
		p.is_not_heavy_hitter = 1;
  }
	sketch_cnt_1[hash1(p.sport, p.dport)] = sketch_cnt_1[hash1(p.sport, p.dport)] + 1;
	sketch_cnt_2[hash2(p.sport, p.dport)] = sketch_cnt_2[hash2(p.sport, p.dport)] + 1;
	sketch_cnt_3[hash3(p.sport, p.dport)] = sketch_cnt_3[hash3(p.sport, p.dport)] + 1;
}
