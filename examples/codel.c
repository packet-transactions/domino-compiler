#define TARGET   5
#define INTERVAL 100

int first_above_time = 0;
int dropping = 0;
int drop_next = 0;
int count = 0;

struct Packet {
  int tick;
  int enq_tick;
  int deq_tick;
  int ok_to_drop;
  int drop;
  int sojourn_time;
};

int mysqrt(int val) {
  // TODO: Fill this up
  return val + 1;
}

void func(struct Packet pkt) {
  // dodeque method
  pkt.sojourn_time = pkt.deq_tick - pkt.enq_tick;
  if (pkt.sojourn_time < TARGET) {
    first_above_time = 0;
  } else {
    if (first_above_time == 0) {
        first_above_time = pkt.tick + INTERVAL;
    } else if (pkt.tick >= first_above_time) {
        pkt.ok_to_drop = 1;
    }
  }

  // Hysterisis
  if (dropping) {
    if (! pkt.ok_to_drop) {
      dropping = 0;
    }
    if (pkt.tick >= drop_next && dropping) {
      pkt.drop = 1;
      count += 1;
      drop_next = drop_next + INTERVAL / mysqrt(count);
    }
  } else if (pkt.ok_to_drop) {
    pkt.drop = 1;
    dropping = 1;
    if (count > 2 && pkt.tick - drop_next < 8 * INTERVAL) {
      count = count - 2;
    } else {
      count = 1;
      drop_next = pkt.tick + INTERVAL / mysqrt(count);
    }
  }
}
