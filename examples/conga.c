// This is a packet reflected back from a destination leaf (d)
// (confusingly called the src field in the packet)
// informing the source leaf (s) what the max util (util) on
// a particular path (path_id) to d is.
// This helps s maintain at any point, the max. util on the best path to d
// and the id of the path to d as well.
struct Packet {
  int util;
  int path_id;
  int src;
  int best_path_util_idx;
  int best_path_idx;
};

int best_path_util[256] = {100}; // Utilization information for each destination, Initially at 100% for everyone.
int best_path[256]      = {-1};  // Next hop / path information for each destination

void func(struct Packet p) {
  p.best_path_util_idx = p.src < 0 ? 0 : p.src % 256;
  p.best_path_idx      = p.src < 0 ? 0 : p.src % 256;
  if (p.util < best_path_util[p.best_path_util_idx]) {
    best_path_util[p.best_path_util_idx] = p.util;
    best_path[p.best_path_idx] = p.path_id;
  } else if (p.path_id == best_path[p.best_path_idx]) {
    best_path_util[p.best_path_util_idx] = p.util;
    // TODO: I guess we aren't switching to another path in
    // case the utilization on the best path went up.
  }
}

// Also, CONGA has multiple transactions, while we are only dealing with
// the hardest of these transactions here.
