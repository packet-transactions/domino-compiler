#include <iostream>
#include <cassert>
#include <stdexcept>
#include <algorithm>
#include "graph.h"
#include "set_idioms.h"

template <class NodeType>
void Graph<NodeType>::add_node(const NodeType & node) {
  // Add node to node_set_
  if (node_set_.find(node) != node_set_.end()) {
    throw std::logic_error("Trying to insert node that already exists in node_set_\n");
  }
  node_set_.insert(node);
  succ_map_[node] = std::vector<NodeType>();
  pred_map_[node] = std::vector<NodeType>();
}

template <class NodeType>
void Graph<NodeType>::add_edge(const NodeType & from_node, const NodeType & to_node) {
  if (node_set_.find(from_node) == node_set_.end()) {
    throw std::logic_error("from_node doesn't exist in node_set_\n");
  }

  if (node_set_.find(to_node) == node_set_.end()) {
    throw std::logic_error("to_node doesn't exist in node_set_\n");
  }

  // If edge already exists, return
  if (std::find(succ_map_.at(from_node).begin(), succ_map_.at(from_node).end(), to_node) != succ_map_.at(from_node).end()) {
    assert(std::find(pred_map_.at(to_node).begin(), pred_map_.at(to_node).end(), from_node) != pred_map_.at(to_node).end());
    std::cout << "Warning: edge already exists, ignoring add_edge command\n";
    return;
  }

  succ_map_.at(from_node).emplace_back(to_node);
  pred_map_.at(to_node).emplace_back(from_node);

  // Keep these lists sorted to ensure the equality comparison works
  std::sort(succ_map_.at(from_node).begin(), succ_map_.at(from_node).end());
  std::sort(pred_map_.at(to_node).begin(), pred_map_.at(to_node).end());
}

template <class NodeType>
bool Graph<NodeType>::operator==(const Graph<NodeType> & b) const {
  return (this->node_set_ == b.node_set_) and
         (this->succ_map_ == b.succ_map_) and
         (this->pred_map_ == b.pred_map_);
}

template <class NodeType>
Graph<NodeType> Graph<NodeType>::operator+(const Graph<NodeType> & b) const {
  if (this->node_set_ != b.node_set_) {
    throw std::logic_error("Graph union is only supported on graphs with identical node sets\n");
  }

  // Copy down nodes, clear out edges
  Graph<NodeType> result = this->copy_and_clear();

  // Start adding edges
  for (const auto & graph : {this, &b})
    for (const auto & node : graph->succ_map_)
      for (const auto & neighbor : node.second)
        result.add_edge(node.first, neighbor);

  return result;
}

template <class NodeType>
Graph<NodeType> Graph<NodeType>::transpose() const {
  Graph transpose_graph = copy_and_clear();

  // Flip edges
  for (const auto & node : succ_map_) {
    for (const auto & neighbor : node.second) {
      // The original graph has an edge from node to neighbor, flip that around
      transpose_graph.succ_map_.at(neighbor).emplace_back(node.first);
      transpose_graph.pred_map_.at(node.first).emplace_back(neighbor);
    }
  }
  return transpose_graph;
}

template <class NodeType>
Graph<NodeType> Graph<NodeType>::copy_and_clear() const {
  Graph copy(*this);
  // Clear out succ_map_ and pred_map_
  for (const auto & node : copy.node_set_) {
    copy.succ_map_.at(node).clear();
    copy.pred_map_.at(node).clear();
  }
  return copy;
}
