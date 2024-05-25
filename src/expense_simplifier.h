#pragma once

#include <vector>

#include "src/debt_graph.h"
#include "src/debts.pb.h"

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
      // The id of one of the neighbors this user is connected to.
      uint64_t neighbor_head_idx;

      // The available capacity for flow of money from this user to the
      // neighbor.
      Cents capacity;
    } neighbor;
  };
};

class ExpenseSimplifier {
 public:
  explicit ExpenseSimplifier(DebtGraph&& graph);

  const DebtGraph& MinimalTransactions() const;

 private:
  // Constructs a layered graph from `source` to `sink` using only edges on the
  // shortest paths from `source` to `sink` in `graph_`.
  std::vector<LayeredGraphNode> ConstructLayeredGraph(uint64_t source,
                                                      uint64_t sink) const;

  DebtGraph simplified_expenses_;

  AugmentedDebtGraph graph_;
};

}  // namespace debt_simpl
