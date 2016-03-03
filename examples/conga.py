best_path_util = [100] * 256
best_path      = [-1]  * 256

def func(p):
  p.best_path_util_idx = p.src
  p.best_path_idx      = p.src
  if (p.util < best_path_util[p.best_path_util_idx]):
    best_path_util[p.best_path_util_idx] = p.util
    best_path[p.best_path_idx] = p.path_id
  elif (p.path_id == best_path[p.best_path_idx]):
    best_path_util[p.best_path_util_idx] = p.util
