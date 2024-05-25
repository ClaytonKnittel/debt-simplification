#include "src/expense_simplifier.h"

#include <cstdint>
#include <deque>
#include <vector>

#include "src/debt_graph.h"

namespace debt_simpl {

bool operator==(const LayeredGraphNode& a, const LayeredGraphNode& b) {
  if (a.type != b.type) {
    return false;
  }
  switch (a.type) {
    case LayeredGraphNodeType::Head: {
      return a.head.id == b.head.id &&
             a.head._internal_level == b.head._internal_level;
    }
    case LayeredGraphNodeType::Neighbor: {
      return a.neighbor.neighbor_head_idx == b.neighbor.neighbor_head_idx &&
             a.neighbor.capacity == b.neighbor.capacity;
    }
    case LayeredGraphNodeType::Tombstone: {
      return true;
    }
  }
}

ExpenseSimplifier::ExpenseSimplifier(DebtGraph&& graph)
    : graph_(std::move(graph)) {}

const DebtGraph& ExpenseSimplifier::MinimalTransactions() const {
  return simplified_expenses_;
}

std::vector<LayeredGraphNode> ExpenseSimplifier::ConstructBlockingFlow(
    uint64_t source, uint64_t sink) const {
  std::vector<LayeredGraphNode> layered_graph;
  std::deque<std::pair<uint64_t, uint32_t>> id_q;
  id_q.push_back({ source, 0 });
  // A map from node id's to first the depth it was discovered at, and later the
  // index in `layered_graph` of the head of the node.
  absl::flat_hash_map<uint64_t, uint64_t> visited_nodes;
  visited_nodes.insert({ source, 0 });
  uint32_t sink_depth = UINT32_MAX;

  while (!id_q.empty()) {
    const auto [node_id, depth] = id_q.front();
    id_q.pop_front();
    // std::cout << "Popped " << node_id << " at depth " << depth << std::endl;

    // Break once we reach the sink depth. No other exploration here is useful
    // since all shortest paths leading to the sink node have already been
    // discovered.
    if (depth == sink_depth) {
      break;
    }

    layered_graph.push_back(LayeredGraphNode{
        .type = LayeredGraphNodeType::Head,
        .head = { .id = node_id, ._internal_level = depth } });
    for (const auto& [neighbor_id, capacity] : graph_.AllDebts(node_id)) {
      if (capacity == 0) {
        continue;
      }

      if (neighbor_id == sink) {
        sink_depth = depth + 1;
      }

      const auto neighbor_it = visited_nodes.find(neighbor_id);
      // std::cout << "  Neighbor " << neighbor_id << std::endl;
      if (neighbor_it != visited_nodes.end()) {
        if (neighbor_it->second != depth + 1) {
          // std::cout << "    Skipping from depth " << neighbor_it->second
          //           << std::endl;
          continue;
        }
      } else {
        visited_nodes.insert({ neighbor_id, static_cast<uint64_t>(depth + 1) });
        id_q.push_back({ neighbor_id, depth + 1 });
      }

      // Temporarily place neighbor_id in place of neighbor_head_idx. Will pass
      // through again at the end and correct all neighbor_head_idx values with
      // the complete visited_nodes map.
      layered_graph.push_back(
          LayeredGraphNode{ .type = LayeredGraphNodeType::Neighbor,
                            .neighbor = { ._internal_neighbor_id = neighbor_id,
                                          .capacity = capacity } });
    }
  }

  // Manually add the sink node since it was never explored.
  if (sink_depth != UINT32_MAX) {
    layered_graph.push_back(LayeredGraphNode{
        .type = LayeredGraphNodeType::Head,
        .head = { .id = sink, ._internal_level = sink_depth } });
  }

  // Remove all visited nodes with depth == sink_depth, except for the sink.
  for (auto it = visited_nodes.begin(); it != visited_nodes.end();) {
    if (it->first != sink && it->second == sink_depth) {
      // std::cout << "erasing " << it->first << std::endl;
      visited_nodes.erase(it++);
    } else {
      it++;
    }
  }

  // Go backwards and mark all nodes to be deleted as tombstones, and remove
  // them from the visited_nodes map if they have no more neighbors.
  uint64_t num_neighbors = 0;
  uint64_t num_tombstones = 0;
  for (auto it = layered_graph.rbegin(); it != layered_graph.rend(); ++it) {
    if (it->type == LayeredGraphNodeType::Head) {
      // std::cout << "head " << it->head.id << " with " << num_neighbors
      //           << " neighbors" << std::endl;
      if (it->head.id != sink && num_neighbors == 0) {
        visited_nodes.erase(it->head.id);
        it->type = LayeredGraphNodeType::Tombstone;
        num_tombstones++;
      } else {
        const auto node_it = visited_nodes.find(it->head.id);
        // Replace the visited_nodes entry with the index of this head assuming
        // all elements will be shifted to the right after removing tombstones.
        uint64_t cur_idx = layered_graph.rend() - it - 1;
        node_it->second = cur_idx + num_tombstones;
      }
      num_neighbors = 0;
    } else {
      const auto neighbor_it =
          visited_nodes.find(it->neighbor._internal_neighbor_id);
      if (neighbor_it == visited_nodes.end()) {
        it->type = LayeredGraphNodeType::Tombstone;
        num_tombstones++;
      } else {
        num_neighbors++;
      }
    }
  }

  // std::cout << "first layered graph:" << std::endl;
  // uint64_t idxx = 0;
  // for (const auto& node : layered_graph) {
  //   switch (node.type) {
  //     case debt_simpl::LayeredGraphNodeType::Head: {
  //       std::cout << idxx << " Head: " << node.head.id << " ("
  //                 << node.head.level << ")" << std::endl;
  //       break;
  //     }
  //     case debt_simpl::LayeredGraphNodeType::Neighbor: {
  //       std::cout << idxx
  //                 << " Neighbor: " << node.neighbor._internal_neighbor_id
  //                 << " (" << node.neighbor.capacity << ")" << std::endl;
  //       break;
  //     }
  //     case debt_simpl::LayeredGraphNodeType::Tombstone: {
  //       std::cout << idxx << " Tombstone" << std::endl;
  //       break;
  //     }
  //   }
  //   idxx++;
  // }

  // std::cout << "Node id to idx off by a little" << std::endl;
  // for (const auto& node : visited_nodes) {
  //   std::cout << "Node: " << node.first << ", " << node.second << " ("
  //             << node.second - num_tombstones << ")" << std::endl;
  // }

  // std::cout << "num tombstones: " << num_tombstones << std::endl;

  // Update all neighbors with neighbor_head_idx to point to the head idx of the
  // neighbor instead of the id, and remove all tombstones.
  for (uint64_t i = 0, j = 0; i < layered_graph.size(); i++) {
    // std::cout << "i: " << i << ", j: " << j << std::endl;
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
        // std::cout << "Updating neighbor " << node.neighbor.neighbor_head_idx
        //           << " to " << node_it->second << " - " << num_tombstones
        //           << std::endl;
        node.neighbor.neighbor_head_idx = node_it->second - num_tombstones;
        break;
      }
      case LayeredGraphNodeType::Tombstone:
        continue;
    }

    layered_graph[j] = node;
    j++;
  }
  layered_graph.resize(layered_graph.size() - num_tombstones);
  layered_graph.shrink_to_fit();

  // Now construct a blocking flow on the graph.
  std::vector<std::pair<uint64_t, uint64_t>> stack;
  while (!stack.empty()) {
    const auto [node_id, max_flow] = stack.back();
    stack.pop_back();
  }

  return layered_graph;
}

}  // namespace debt_simpl
