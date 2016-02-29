count = 0
N = 100
def func(pkt) :
  if (count == N - 1) :
    pkt.sample = 1
    count = 0
  else :
    pkt.sample = 0
    count = count + 1
