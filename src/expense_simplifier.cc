#include "src/expense_simplifier.h"

#include "src/debt_graph.h"
#include "src/layered_graph.h"

namespace debt_simpl {

ExpenseSimplifier::ExpenseSimplifier(DebtGraph&& graph)
    : simplified_expenses_(std::move(graph)) {
  AugmentedDebtGraph augmented_graph(simplified_expenses_);
  simplified_expenses_.Clear();
  BuildMinimalTransactions(std::move(augmented_graph));
}

const DebtGraph& ExpenseSimplifier::MinimalTransactions() const {
  return simplified_expenses_;
}

void ExpenseSimplifier::BuildMinimalTransactions(AugmentedDebtGraph&& graph) {
  std::vector<DebtGraphEdge> edges = graph.AllDebts();
  std::sort(edges.begin(), edges.end(),
            [&graph](const DebtGraphEdge& e1, const DebtGraphEdge& e2) {
              int64_t cmp_key1 = std::abs(graph.TotalDebt(e1.lender_id) -
                                          graph.TotalDebt(e1.receiver_id));
              int64_t cmp_key2 = std::abs(graph.TotalDebt(e2.lender_id) -
                                          graph.TotalDebt(e2.receiver_id));
              return cmp_key1 == cmp_key2 ? e1.debt < e2.debt
                                          : cmp_key1 < cmp_key2;
            });

  while (!edges.empty()) {
    const DebtGraphEdge edge = edges.back();
    edges.pop_back();

    const uint64_t lender_id = edge.lender_id;
    const uint64_t receiver_id = edge.receiver_id;

    const Cents debt = graph.Debt(lender_id, receiver_id);
    if (debt == 0) {
      continue;
    }

    uint64_t total_flow = 0;
    while (true) {
      const LayeredGraph blocking_flow =
          LayeredGraph::ConstructBlockingFlow(graph, receiver_id, lender_id);
      if (blocking_flow.size() == 0) {
        break;
      }

      uint64_t payer_id;
      for (const LayeredGraphNode& node : blocking_flow) {
        if (node.type == LayeredGraphNodeType::Head) {
          payer_id = node.head.id;
          continue;
        }

        const uint64_t neighbor_id =
            blocking_flow[node.neighbor.neighbor_head_idx].head.id;
        graph.PushFlow(neighbor_id, payer_id, node.neighbor.flow);
      }

      total_flow += blocking_flow.ComputeFlow();
    }

    graph.EraseEdge(lender_id, receiver_id);
    simplified_expenses_.PushFlow(receiver_id, lender_id, total_flow);
  }
}

}  // namespace debt_simpl
