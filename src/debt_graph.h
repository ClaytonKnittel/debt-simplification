#pragma once

#include <string>

#include "absl/container/flat_hash_map.h"

typedef uint64_t Cents;

class DebtGraphNode {
 public:
  explicit DebtGraphNode(uint64_t id);

  void AddDebt(uint64_t owee_id, Cents amount);

 private:
  uint64_t id_;
  absl::flat_hash_map<uint64_t, Cents> debts_;
};

class DebtGraph {
 private:
  // Map of user names to unique id's.
  absl::flat_hash_map<std::string, uint64_t> id_map_;

  // List of all nodes of the graph. A user's id is the index into this list
  // where their corresponding node is.
  std::vector<DebtGraphNode> node_list_;
};
