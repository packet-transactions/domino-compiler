#define EWMA_GAIN 10
#define SCALE 100
int current_val = 0;
int initialized = 0;

struct Packet {
  int val;
  int new_val;
  int tmp_scaled_val;
};

void func(struct Packet p) {
  if (! initialized) {
   current_val = p.val;
   initialized = 1;
  } else {
   current_val = (current_val * EWMA_GAIN + (SCALE - EWMA_GAIN) * p.val) / SCALE;
  }
  p.new_val = current_val;
}
