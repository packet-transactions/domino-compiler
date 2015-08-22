#define PBS 2000000000
#define CBS 2000000000

#define PIR 2
#define CIR 1

struct Packet {
  int size;
  int color;
  int time;
};

int tp = PBS;
int tc = CBS;
int last_time = 0;

void func(struct Packet pkt) {
  if (tp < pkt.size) {
    pkt.color = 1;
  } else if (tc < pkt.size) {
    pkt.color = 2;
    tp = tp - pkt.size;
  } else {
    pkt.color = 3;
    tp = tp - pkt.size;
    tc = tc - pkt.size;
  }

  // Refill logic
  tp = tp + PIR * (pkt.time - last_time);
  if (tp > PBS) tp = PBS;

  tc = tc + CIR * (pkt.time - last_time);
  if (tc > PBS) tc = CBS;

  last_time = pkt.time;

}
