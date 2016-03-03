PBS=2000000000
CBS=2000000000
PIR=2
CIR=1
tp=PBS
tc=CBS
last_time=0

def func(pkt):
  if (tp < pkt.size):
    pkt.color = 1
  elif (tc < pkt.size):
    pkt.color = 2
    tp = tp - pkt.size
  else:
    pkt.color = 3
    tp = tp - pkt.size
    tc = tc - pkt.size

  tp = tp + PIR * (pkt.time - last_time)
  if (tp > PBS):
    tp = PBS

  tc = tc + CIR * (pkt.time - last_time)
  if (tc > PBS):
    tc = CBS

  last_time = pkt.time
