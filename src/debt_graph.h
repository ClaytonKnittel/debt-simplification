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

  Cents Debt(uint64_t ower_id) const;

  void ClearCredits();

  const absl::flat_hash_map<uint64_t, Cents>& AllDebts() const;

 private:
  absl::flat_hash_map<uint64_t, Cents> debts_;
};

class DebtGraphInternal {
 public:
  DebtGraphInternal() = default;

  DebtGraphInternal(const DebtGraphInternal&) = default;
  DebtGraphInternal& operator=(const DebtGraphInternal&) = default;

  // Returns the total number of users in the graph. Id's will span the range
  // [0, NumUsers()).
  uint64_t NumUsers() const;

  // Returns the flow of money from `receiver_id` to `lender_id`.
  Cents Debt(uint64_t lender_id, uint64_t receiver_id) const;

  // Pushes flow of money from `from` to `to`. This adds `amount` debt owed to
  // `to` by `from`. This can be used to offset debt `from` owes `to`.
  void PushFlow(uint64_t from, uint64_t to, Cents amount);

  // Returns a map of all user id's that `user_id` is indebted to and how much
  // each debt is.
  const absl::flat_hash_map<uint64_t, Cents>& AllDebts(uint64_t user_id) const;

 protected:
  // Adds a new user and returns their ID.
  uint64_t AddNewUser();

  // Clears all negative debt owed to `lender_id`.
  void ClearCredits(uint64_t lender_id);

 private:
  // Adds debt that `lender_id` is owed from `receiver_id`.
  void AddDebt(uint64_t lender_id, uint64_t receiver_id, Cents amount);

  // List of all nodes of the graph. A user's id is the index into this list
  // where their corresponding node is.
  std::vector<DebtGraphNode> node_list_;
};

class DebtGraph : public DebtGraphInternal {
 public:
  DebtGraph() = default;

  DebtGraph(const DebtGraph&) = default;
  DebtGraph& operator=(const DebtGraph&) = default;

  static absl::StatusOr<DebtGraph> BuildFromProto(const DebtList& debt_list);

  // Returns the amount of money `receiver` owes `lender`.
  absl::StatusOr<Cents> AmountOwed(absl::string_view lender,
                                   absl::string_view receiver) const;

  absl::Status AddTransaction(const Transaction& t);

 private:
  // Given a user's name, returns the unique id of the user, or an error if
  // that user doesn't exist.
  absl::StatusOr<uint64_t> FindUserId(absl::string_view username) const;

  // Given a user's name, returns the unique id of the user, creating a new
  // username-id binding if one doesn't already exist for this user.
  uint64_t FindOrAssignUserId(std::string username);

  // Map of user names to unique id's.
  absl::flat_hash_map<std::string, uint64_t> id_map_;
};

class AugmentedDebtGraph : public DebtGraphInternal {
 public:
  AugmentedDebtGraph() = default;

  AugmentedDebtGraph(const AugmentedDebtGraph&) = default;
  AugmentedDebtGraph& operator=(const AugmentedDebtGraph&) = default;

  // Constructs an AugmentedDebtGraph from a DebtGraph, initializing all
  // backwards edges to 0.
  AugmentedDebtGraph(DebtGraph&& graph);

 private:
  // Clears all credits recorded in the graph, which is useful when constructing
  // an AugmentedDebtGraph from a DebtGraph.
  void ClearCredits();
};

}  // namespace debt_simpl
