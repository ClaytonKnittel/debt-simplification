#include "src/debt_graph.h"

#include <string>

#include "absl/status/statusor.h"
#include "absl/strings/str_format.h"
#include "absl/strings/string_view.h"

#include "src/debts.pb.h"
#include "src/utils.h"

namespace debt_simpl {

void DebtGraphNode::AddDebt(uint64_t ower_id, Cents amount) {
  debts_[ower_id] += amount;
}

void DebtGraphNode::AddCredit(uint64_t owee_id, Cents amount) {
  debts_[owee_id] -= amount;
}

Cents DebtGraphNode::Debt(uint64_t ower_id) const {
  const auto it = debts_.find(ower_id);
  if (it == debts_.cend()) {
    return 0;
  } else {
    return it->second;
  }
}

// static
absl::StatusOr<DebtGraph> DebtGraph::BuildFromProto(const DebtList& debt_list) {
  DebtGraph graph;

  for (const Transaction& t : debt_list.transactions()) {
    RETURN_IF_ERROR(graph.AddTransaction(t));
  }

  return graph;
}

absl::StatusOr<Cents> DebtGraph::AmountOwed(absl::string_view lender,
                                            absl::string_view receiver) const {
  uint64_t lender_id, receiver_id;
  ASSIGN_OR_RETURN(lender_id, FindUserId(lender));
  ASSIGN_OR_RETURN(receiver_id, FindUserId(receiver));

  return node_list_[receiver_id].Debt(lender_id);
}

absl::StatusOr<uint64_t> DebtGraph::FindUserId(
    absl::string_view username) const {
  const auto it = id_map_.find(username);
  if (it == id_map_.end()) {
    return absl::InternalError(absl::StrFormat("No such user %s", username));
  }
  return it->second;
}

uint64_t DebtGraph::FindOrAssignUserId(std::string username) {
  const auto [it, inserted] =
      id_map_.insert({ username, static_cast<uint64_t>(id_map_.size()) });
  if (inserted) {
    node_list_.emplace_back();
  }
  return it->second;
}

absl::Status DebtGraph::AddTransaction(const Transaction& t) {
  uint64_t lender_id = FindOrAssignUserId(t.lender());
  uint64_t receiver_id = FindOrAssignUserId(t.receiver());

  node_list_[lender_id].AddCredit(receiver_id, t.cents());
  node_list_[receiver_id].AddDebt(lender_id, t.cents());
  return absl::OkStatus();
}

}  // namespace debt_simpl
