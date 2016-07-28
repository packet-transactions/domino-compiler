NUM_ENTRIES = 256
filter1 = [0] * NUM_ENTRIES
filter2 = [0] * NUM_ENTRIES
filter3 = [0] * NUM_ENTRIES

def func(pkt):
  pkt.filter1_idx = hash2a(pkt.sport, pkt.dport) % NUM_ENTRIES;
  pkt.filter2_idx = hash2b(pkt.sport, pkt.dport) % NUM_ENTRIES;
  pkt.filter3_idx = hash2c(pkt.sport, pkt.dport) % NUM_ENTRIES;
  pkt.member = (filter1[pkt.filter1_idx] and \
                filter2[pkt.filter2_idx] and \
                filter3[pkt.filter3_idx]);
  filter1[pkt.filter1_idx] = 1;
  filter2[pkt.filter2_idx] = 1;
  filter3[pkt.filter3_idx] = 1;
