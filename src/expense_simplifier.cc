#include "src/expense_simplifier.h"

#include <cstdint>
#include <deque>
#include <optional>
#include <vector>

#include "src/debt_graph.h"

namespace debt_simpl {

ExpenseSimplifier::ExpenseSimplifier(DebtGraph&& graph)
    : graph_(std::move(graph)) {}

const DebtGraph& ExpenseSimplifier::MinimalTransactions() const {
  return simplified_expenses_;
}

std::vector<LayeredGraphNode> ExpenseSimplifier::ConstructLayeredGraph(
    uint64_t source, uint64_t sink) const {
  std::vector<LayeredGraphNode> layered_graph;
  std::deque<std::pair<uint64_t, uint32_t>> id_q;
  id_q.push_back({ source, 0 });
  uint32_t sink_dist = UINT32_MAX;
  // A map from node id's to the index in `layered_graph` of the head of the
  // node.
  absl::flat_hash_map<uint64_t, std::optional<uint64_t>> visited_nodes;

  while (!id_q.empty()) {
    const auto [node_id, depth] = id_q.front();
    id_q.pop_front();

    if (depth > sink_dist) {
      // Stop once we have passed the sink.
      break;
    } else if (node_id == sink) {
      sink_dist = depth;
    }

    layered_graph.push_back(
        LayeredGraphNode{ .type = LayeredGraphNodeType::Head,
                          .head = { .id = node_id, .level = depth } });
    for (const auto& [neighbor_id, capacity] : graph_.AllDebts(node_id)) {
      if (capacity == 0) {
        continue;
      }

      layered_graph.push_back(LayeredGraphNode{
          .type = LayeredGraphNodeType::Neighbor,
          .neighbor = { .id = neighbor_id, .capacity = capacity } });
    }

    // If we did not add any edges from this node, remove it from the graph.
    if (layered_graph.back().type == LayeredGraphNodeType::Head) {
      layered_graph.pop_back();
    }
  }
}

}  // namespace debt_simpl
