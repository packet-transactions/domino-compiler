// Sample every 100th packet in a flow
#define N 100

typedef struct Packet {
  int sample;
} Packet;

int count = 0;

void func(Packet pkt) {
  if (count == N - 1) {
    pkt.sample = 1;
    count = 0;
  } else {
    pkt.sample = 0;
    count = count + 1;
  }
}
