#pragma once

#include <string>

#include "absl/container/flat_hash_map.h"
#include "absl/status/statusor.h"

#include "src/debts.pb.h"

namespace debt_simpl {

typedef uint64_t Cents;

class DebtGraphNode {
 public:
  DebtGraphNode() = default;

  void AddDebt(uint64_t owee_id, Cents amount);

 private:
  absl::flat_hash_map<uint64_t, Cents> debts_;
};

class DebtGraph {
 public:
  DebtGraph() = default;

  static absl::StatusOr<DebtGraph> BuildFromProto(const DebtList& debt_list);

 private:
  absl::Status AddTransaction(const Transaction& t);

  // Map of user names to unique id's.
  absl::flat_hash_map<std::string, uint64_t> id_map_;

  // List of all nodes of the graph. A user's id is the index into this list
  // where their corresponding node is.
  std::vector<DebtGraphNode> node_list_;
};

}  // namespace debt_simpl
