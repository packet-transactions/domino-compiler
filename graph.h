#ifndef GRAPH_H_
#define GRAPH_H_

#include <map>
#include <set>
#include <vector>
#include <ostream>
#include <algorithm>
#include <functional>
#include <string>
#include <cassert>
#include <stdexcept>
#include "set_idioms.h"

/// Adjacency list representation of graph
/// Store both out and in edges
template <class NodeType>
class Graph {
 public:
  /// Graph constructor, taking node printer as optional argument
  Graph<NodeType>(const std::function<std::string(const NodeType)> & node_printer)
    : node_printer_(node_printer) {};

  /// Add node alone to existing graph, check that node doesn't already exist
  void add_node(const NodeType & node);

  /// Add edge to existing graph, check that both from_node and to_node exist
  void add_edge(const NodeType & from_node, const NodeType & to_node);

  /// Find the graph transpose G', i.e. for every edge u-->v in G,
  /// there is an edge v-->u in G'
  /// Returning a Graph is ok because of C++11's move semantics
  /// and the likely move elision from the compiler
  Graph<NodeType> transpose() const;

  /// Copy over graph and clear out all edges
  Graph<NodeType> copy_and_clear() const;

  /// Print graph to stream using Graphviz's dot format
  friend std::ostream & operator<< (std::ostream & out, const Graph<NodeType> & graph) {
    assert (graph.node_printer_);

    out << "digraph graph_output {node [shape = box];\n";
    for (const auto & node : graph.node_set_) {
      out << hash_string(graph.node_printer_(node)) << " [label = \"" << graph.node_printer_(node)  << "\" ];\n";
    }

    for (const auto & node_pair : graph.succ_map_)
      for (const auto & neighbor : node_pair.second)
        out << hash_string(graph.node_printer_(node_pair.first)) << " -> "
            << hash_string(graph.node_printer_(neighbor)       ) << " ;\n";
    out << "}";

    return out;
  }

  /// Used for unit tests that check expected graph output
  bool operator==(const Graph<NodeType> & b) const;

  /// Graph union,
  /// we restrict unions to graphs with identical vertex sets
  Graph<NodeType> operator+(const Graph<NodeType> & b) const;

  /// Accessors
  const auto & node_set() const { return node_set_; }
  const auto & succ_map() const { return succ_map_; }
  const auto & pred_map() const { return pred_map_; }

  /// Check if an edge exists from a to b
  bool exists_edge(const NodeType & a, const NodeType & b) const {
    return (std::find(succ_map_.at(a).begin(), succ_map_.at(a).end(), b) != succ_map_.at(a).end() and
            std::find(pred_map_.at(b).begin(), pred_map_.at(b).end(), a) != pred_map_.at(b).end());
  }


 private:
  /// Set of all nodes in the graph
  std::set<NodeType> node_set_ = {};

  /// Successor map from a node to all successor nodes (outgoing edges)
  std::map<NodeType, std::vector<NodeType>> succ_map_ = {};

  /// Predecessor map from a node to all predecessor nodes (incoming edges)
  std::map<NodeType, std::vector<NodeType>> pred_map_ = {};

  /// Node printer function
  std::function<std::string(const NodeType &)> node_printer_;

  /// Hash string into unique ID
  static std::string hash_string (const std::string & str) { return std::to_string(std::hash<std::string>()(str)); }
};

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

#endif  // GRAPH_H_
