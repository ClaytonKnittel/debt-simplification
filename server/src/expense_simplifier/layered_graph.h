#pragma once

#include <ostream>
#include <stdint.h>
#include <vector>

#include "server/src/expense_simplifier/debt_graph.h"

namespace debt_simpl {

enum class LayeredGraphNodeType {
  Head,
  Neighbor,
  // Used temporarily in construction of the layered graph to mark a node as
  // ready for deletion.
  Tombstone,
};

struct LayeredGraphNode {
  LayeredGraphNodeType type;

  union {
    // Members are defined if `type` == Head.
    struct {
      // The user id for this node. The subsequent nodes of type `Neighbor` are
      // the neighbors of this user.
      uint64_t id;

      // The level of this node, i.e. the distance between this node and the
      // source node. This == 0 for the source node.
      uint32_t level;
    } head;

    // Members are defined if `type` == Neighbor.
    struct {
      union {
        // The id of the neighbor. This is only meaningful during construction
        // of the layered graph, and will not be defined in the graph produced
        // by `ConstructLayeredGraph()`.
        uint64_t _internal_neighbor_id;

        // The index in the layered graph of the head of one of the neighbors
        // this user is connected to.
        uint64_t neighbor_head_idx;
      };

      // The available capacity for flow of money from this user to the
      // neighbor.
      Cents capacity;

      // The flow of money from this node to the neighbor.
      Cents flow;
    } neighbor;
  };
};

bool operator==(const LayeredGraphNode& a, const LayeredGraphNode& b);

class LayeredGraph {
 public:
  LayeredGraphNode& operator[](size_t i);
  const LayeredGraphNode& operator[](size_t i) const;

  size_t size() const;

  std::vector<LayeredGraphNode>::iterator begin();
  std::vector<LayeredGraphNode>::iterator end();

  std::vector<LayeredGraphNode>::const_iterator begin() const;
  std::vector<LayeredGraphNode>::const_iterator end() const;

  const std::vector<LayeredGraphNode>& NodeList() const;

  // Constructs a layered graph from `source` to `sink` using only edges on the
  // shortest paths from `source` to `sink` in `graph_`, then computes a
  // blocking flow on the resulting DAG.
  //
  // The return value is a vector of `LayeredGraphNode`s, which are either
  // `Header`s or `Neighbor`s. Each header is followed by some number of
  // neighbors, which are the corresponding neighbors of the node associated
  // with that header. Those neighbor nodes point to the index into this vector
  // of the header of the neighbor node.
  static LayeredGraph ConstructBlockingFlow(const AugmentedDebtGraph& graph,
                                            uint64_t source, uint64_t sink);

  // Computes the total flow of money in this graph.
  Cents ComputeFlow() const;

 private:
  LayeredGraph() = default;

  std::vector<LayeredGraphNode> nodes_;
};

std::ostream& operator<<(std::ostream&, const LayeredGraph&);

}  // namespace debt_simpl
