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
};

int best_path_util[256] = {100}; // Utilization information for each destination, Initially at 100% for everyone.
int best_path[256]      = {-1};  // Next hop / path information for each destination

void func(struct Packet p) {
  if (p.util < best_path_util[p.src % 256]) {
    best_path_util[p.src % 256] = p.util;
    best_path[p.src % 256] = p.path_id;
  } else if (p.path_id == best_path[p.src]) {
    best_path_util[p.src % 256] = p.util;
    // TODO: I guess we aren't switching to another path in
    // case the utilization on the best path went up.
  }
}

// Also, CONGA has multiple transactions, while we are only dealing with
// the hardest of these transactions here.
