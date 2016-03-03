TARGET   = 5
INTERVAL = 100
first_above_time = 0
dropping = 0
drop_next = 0
count = 0

def func(pkt):
  pkt.sojourn_time = pkt.deq_tick - pkt.enq_tick
  if (pkt.sojourn_time < TARGET):
    first_above_time = 0
  else:
    if (first_above_time == 0):
      first_above_time = pkt.tick + INTERVAL
    elif (pkt.tick >= first_above_time):
      pkt.ok_to_drop = 1

  if (dropping):
    if (not pkt.ok_to_drop):
      dropping = 0
    if (pkt.tick >= drop_next and dropping):
      pkt.drop = 1
      count += 1
      drop_next = drop_next + INTERVAL / mysqrt(count)
  elif (pkt.ok_to_drop):
    pkt.drop = 1
    dropping = 1
    if (count > 2 and pkt.tick - drop_next < 8 * INTERVAL):
      count = count - 2
    else:
      count = 1
      drop_next = pkt.tick + INTERVAL / mysqrt(count)
