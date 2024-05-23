#include "src/debt_graph.h"

#include "absl/status/statusor.h"

#include "src/debts.pb.h"
#include "src/utils.h"

namespace debt_simpl {

void DebtGraphNode::AddDebt(uint64_t owee_id, Cents amount) {
  debts_[owee_id] += amount;
}

// static
absl::StatusOr<DebtGraph> DebtGraph::BuildFromProto(const DebtList& debt_list) {
  DebtGraph graph;

  for (const Transaction& t : debt_list.transactions()) {
    RETURN_IF_ERROR(graph.AddTransaction(t));
  }

  return graph;
}

absl::Status DebtGraph::AddTransaction(const Transaction& t) {
  return absl::OkStatus();
}

}  // namespace debt_simpl
