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
  // A map from node id's to first the depth it was discovered at, and later the
  // index in `layered_graph` of the head of the node.
  absl::flat_hash_map<uint64_t, uint64_t> visited_nodes;
  visited_nodes.insert({ source, 0 });

  while (!id_q.empty()) {
    const auto [node_id, depth] = id_q.front();
    id_q.pop_front();

    layered_graph.push_back(
        LayeredGraphNode{ .type = LayeredGraphNodeType::Head,
                          .head = { .id = node_id, .level = depth } });
    for (const auto& [neighbor_id, capacity] : graph_.AllDebts(node_id)) {
      if (capacity == 0) {
        continue;
      }

      const auto neighbor_it = visited_nodes.find(neighbor_id);
      if (neighbor_it != visited_nodes.end()) {
        if (neighbor_it->second != depth + 1) {
          continue;
        }
      } else {
        visited_nodes.insert({ neighbor_id, static_cast<uint64_t>(depth + 1) });
      }

      // Temporarily place neighbor_id in place of neighbor_head_idx. Will pass
      // through again at the end and correct all neighbor_head_idx values with
      // the complete visited_nodes map.
      layered_graph.push_back(
          LayeredGraphNode{ .type = LayeredGraphNodeType::Neighbor,
                            .neighbor = { ._internal_neighbor_id = neighbor_id,
                                          .capacity = capacity } });
    }

    // If we did not add any edges from this node, remove it from the graph.
    if (node_id != sink &&
        layered_graph.back().type == LayeredGraphNodeType::Head) {
      layered_graph.pop_back();
    }

    if (node_id == sink) {
      break;
    }
  }

  // Go backwards and mark all nodes to be deleted as tombstones, and remove
  // them from the visited_nodes map if they have no more neighbors.
  uint64_t num_neighbors = 0;
  uint64_t num_tombstones = 0;
  for (auto it = layered_graph.rbegin(); it != layered_graph.rend(); ++it) {
    if (it->type == LayeredGraphNodeType::Head) {
      if (it->head.id != sink && num_neighbors == 0) {
        visited_nodes.erase(it->head.id);
        it->type = LayeredGraphNodeType::Tombstone;
        num_tombstones++;
      } else {
        const auto node_it = visited_nodes.find(it->head.id);
        // Replace the visited_nodes entry with the index of this head assuming
        // all elements will be shifted to the right after removing tombstones.
        uint64_t cur_idx = it - layered_graph.rend() - 1;
        node_it->second = cur_idx + num_tombstones;
      }
      num_neighbors = 0;
    } else {
      if (!visited_nodes.contains(it->neighbor._internal_neighbor_id)) {
        it->type = LayeredGraphNodeType::Tombstone;
        num_tombstones++;
      } else {
        num_neighbors++;
      }
    }
  }

  // Update all neighbors with neighbor_head_idx to point to the head idx of the
  // neighbor instead of the id, and remove all tombstones.
  for (uint64_t i = 0, j = 0; i < layered_graph.size(); i++) {
    LayeredGraphNode node = layered_graph.at(i);
    switch (node.type) {
      case LayeredGraphNodeType::Head:
        break;
      case LayeredGraphNodeType::Neighbor: {
        const auto node_it =
            visited_nodes.find(node.neighbor._internal_neighbor_id);
        // Offset the index of each neighbor, which assumed all nodes would
        // shift to the right after removal of tombstones, by the total number
        // of tombstones, which is the difference in indices between shifting to
        // the right and shifting to the left.
        node.neighbor.neighbor_head_idx = node_it->second - num_tombstones;
        break;
      }
      case LayeredGraphNodeType::Tombstone:
        continue;
    }

    layered_graph[j] = node;
    j++;
  }

  return layered_graph;
}

}  // namespace debt_simpl
