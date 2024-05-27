#include "src/expense_simplifier/debt_graph.h"

#include <algorithm>
#include <cstdint>
#include <string>

#include "absl/status/statusor.h"
#include "absl/strings/str_format.h"
#include "absl/strings/string_view.h"

#include "src/expense_simplifier/debts.pb.h"
#include "src/expense_simplifier/utils.h"

namespace debt_simpl {

void DebtGraphNode::AddDebt(uint64_t ower_id, Cents amount) {
  debts_[ower_id] += amount;
  total_debt_ += amount;
}

Cents DebtGraphNode::Debt(uint64_t user_id) const {
  const auto it = debts_.find(user_id);
  if (it == debts_.cend()) {
    return 0;
  } else {
    return it->second;
  }
}

Cents DebtGraphNode::TotalDebt() const {
  return total_debt_;
}

void DebtGraphNode::ClearCredits() {
  // Erase all negative debts, which are credits. Do not modify total_debt_,
  // since this method is only used when translating a graph to an augmented
  // graph.
  for (auto it = debts_.begin(); it != debts_.end();) {
    if (it->second <= 0) {
      debts_.erase(it++);
    } else {
      it++;
    }
  }
}

void DebtGraphNode::EraseDebt(uint64_t id) {
  const auto it = debts_.find(id);
  if (it != debts_.end()) {
    total_debt_ -= it->second;
    debts_.erase(it);
  }
}

void DebtGraphNode::Clear() {
  debts_.clear();
  total_debt_ = 0;
}

const absl::flat_hash_map<uint64_t, Cents>& DebtGraphNode::AllDebts() const {
  return debts_;
}

uint64_t DebtGraphInternal::NumUsers() const {
  return static_cast<uint64_t>(node_list_.size());
}

Cents DebtGraphInternal::Debt(uint64_t receiver_id, uint64_t lender_id) const {
  return node_list_[receiver_id].Debt(lender_id);
}

Cents DebtGraphInternal::TotalDebt(uint64_t id) const {
  return node_list_[id].TotalDebt();
}

void DebtGraphInternal::PushFlow(uint64_t from, uint64_t to, Cents amount) {
  AddDebt(from, to, amount);
  AddDebt(to, from, -amount);
}

void DebtGraphInternal::EraseEdge(uint64_t user1_id, uint64_t user2_id) {
  node_list_[user1_id].EraseDebt(user2_id);
  node_list_[user2_id].EraseDebt(user1_id);
}

void DebtGraphInternal::Clear() {
  for (DebtGraphNode& node : node_list_) {
    node.Clear();
  }
}

const absl::flat_hash_map<uint64_t, Cents>& DebtGraphInternal::AllDebts(
    uint64_t user_id) const {
  return node_list_[user_id].AllDebts();
}

const std::vector<DebtGraphEdge> DebtGraphInternal::AllDebts() const {
  std::vector<DebtGraphEdge> edges;
  for (uint64_t receiver_id = 0; receiver_id < node_list_.size();
       receiver_id++) {
    const DebtGraphNode& node = node_list_[receiver_id];
    for (const auto [lender_id, debt] : node.AllDebts()) {
      edges.push_back(DebtGraphEdge{
          .receiver_id = receiver_id, .lender_id = lender_id, .debt = debt });
    }
  }
  return edges;
}

uint64_t DebtGraphInternal::AddNewUser() {
  uint64_t id = static_cast<uint64_t>(node_list_.size());
  node_list_.emplace_back();
  return id;
}

void DebtGraphInternal::ClearCredits(uint64_t lender_id) {
  node_list_[lender_id].ClearCredits();
}

void DebtGraphInternal::AddDebt(uint64_t lender_id, uint64_t receiver_id,
                                Cents amount) {
  node_list_[receiver_id].AddDebt(lender_id, amount);
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

  return Debt(receiver_id, lender_id);
}

absl::StatusOr<Cents> DebtGraph::TotalDebt(absl::string_view user) const {
  DEFINE_OR_RETURN(uint64_t, id, FindUserId(user));
  return DebtGraphInternal::TotalDebt(id);
}

absl::Status DebtGraph::AddTransaction(const Transaction& t) {
  uint64_t lender_id = FindOrAssignUserId(t.lender());
  uint64_t receiver_id = FindOrAssignUserId(t.receiver());

  PushFlow(lender_id, receiver_id, t.cents());
  return absl::OkStatus();
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
    AddNewUser();
  }
  return it->second;
}

AugmentedDebtGraph::AugmentedDebtGraph(const DebtGraph& graph)
    : DebtGraphInternal(graph) {
  ClearCredits();
}

void AugmentedDebtGraph::ClearCredits() {
  uint64_t num_users = NumUsers();
  for (uint64_t id = 0; id < num_users; id++) {
    DebtGraphInternal::ClearCredits(id);
  }
}

}  // namespace debt_simpl