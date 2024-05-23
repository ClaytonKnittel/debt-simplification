#pragma once

#include <string>

#include "absl/container/flat_hash_map.h"
#include "absl/status/statusor.h"
#include "absl/strings/string_view.h"

#include "src/debts.pb.h"

namespace debt_simpl {

typedef int64_t Cents;

class DebtGraphNode {
 public:
  DebtGraphNode() = default;

  void AddDebt(uint64_t ower_id, Cents amount);

  void AddCredit(uint64_t owee_id, Cents amount);

  Cents Debt(uint64_t ower_id) const;

 private:
  absl::flat_hash_map<uint64_t, Cents> debts_;
};

class DebtGraph {
 public:
  static absl::StatusOr<DebtGraph> BuildFromProto(const DebtList& debt_list);

  // Returns the amount of money `receiver` owes `lender`.
  absl::StatusOr<Cents> AmountOwed(absl::string_view lender,
                                   absl::string_view receiver) const;

 private:
  DebtGraph() = default;

  // Given a user's name, returns the unique id of the user, or an error if
  // that user doesn't exist.
  absl::StatusOr<uint64_t> FindUserId(absl::string_view username) const;

  // Given a user's name, returns the unique id of the user, creating a new
  // username-id binding if one doesn't already exist for this user.
  uint64_t FindOrAssignUserId(std::string username);

  absl::Status AddTransaction(const Transaction& t);

  // Map of user names to unique id's.
  absl::flat_hash_map<std::string, uint64_t> id_map_;

  // List of all nodes of the graph. A user's id is the index into this list
  // where their corresponding node is.
  std::vector<DebtGraphNode> node_list_;
};

}  // namespace debt_simpl
