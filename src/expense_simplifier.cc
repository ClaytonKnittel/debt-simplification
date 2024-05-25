#include "src/expense_simplifier.h"

#include <cstdint>
#include <deque>
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
  // A map from node id's to the index in `layered_graph` of the head of the
  // node.
  absl::flat_hash_map<uint64_t, uint64_t> visited_nodes;

  while (!id_q.empty()) {
    const auto [node_id, depth] = id_q.front();
    id_q.pop_front();

    uint64_t head_idx = static_cast<uint64_t>(layered_graph.size());
    layered_graph.push_back(
        LayeredGraphNode{ .type = LayeredGraphNodeType::Head,
                          .head = { .id = node_id, .level = depth } });
    for (const auto& [neighbor_id, capacity] : graph_.AllDebts(node_id)) {
      if (capacity == 0) {
        continue;
      }

      // Temporarily place neighbor_id in place of neighbor_head_idx. Will pass
      // through again at the end and correct all neighbor_head_idx values with
      // the complete visited_nodes map.
      layered_graph.push_back(
          LayeredGraphNode{ .type = LayeredGraphNodeType::Neighbor,
                            .neighbor = { .neighbor_head_idx = neighbor_id,
                                          .capacity = capacity } });
    }

    // If we did not add any edges from this node, remove it from the graph.
    if (node_id != sink &&
        layered_graph.back().type == LayeredGraphNodeType::Head) {
      layered_graph.pop_back();
    } else {
      visited_nodes.insert({ node_id, head_idx });
    }

    if (node_id == sink) {
      break;
    }
  }

  // Go backwards and mark all nodes to be deleted as tombstones, and remove
  // them from the visited_nodes map if they have no more neighbors.
  uint64_t num_neighbors = 0;
  for (auto it = layered_graph.rbegin(); it != layered_graph.rend(); ++it) {
    if (it->type == LayeredGraphNodeType::Head) {
      if (it->head.id != sink && num_neighbors == 0) {
        visited_nodes.erase(it->head.id);
        it->type = LayeredGraphNodeType::Tombstone;
      }
      num_neighbors = 0;
    } else {
      num_neighbors++;
      uint64_t neighbor_id = it->neighbor.neighbor_head_idx;
      const auto nodes_it = visited_nodes.find(neighbor_id);
      if (nodes_it == visited_nodes.end()) {
        it->type = LayeredGraphNodeType::Tombstone;
      } else {
        it->neighbor.neighbor_head_idx = nodes_it->second;
      }
    }
  }

  // Update all neighbor_head_idx to point to the head idx of the neighobr, and
  // remove all neighbors which were never fully explored.
  layered_graph.erase(std::remove_if(layered_graph.begin(), layered_graph.end(),
                                     [](LayeredGraphNode& node) {
                                       return node.type ==
                                              LayeredGraphNodeType::Tombstone;
                                     }),
                      layered_graph.end());

  return layered_graph;
}

}  // namespace debt_simpl
