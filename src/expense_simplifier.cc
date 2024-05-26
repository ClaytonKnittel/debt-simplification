#include "src/expense_simplifier.h"

#include "src/debt_graph.h"
#include "src/layered_graph.h"

namespace debt_simpl {

ExpenseSimplifier::ExpenseSimplifier(DebtGraph&& graph)
    : simplified_expenses_(std::move(graph)) {
  AugmentedDebtGraph augmented_graph(simplified_expenses_);
}

const DebtGraph& ExpenseSimplifier::MinimalTransactions() const {
  return simplified_expenses_;
}

void ExpenseSimplifier::BuildMinimalTransactions(AugmentedDebtGraph&& graph) {
  std::vector<DebtGraphEdge> edges = graph.AllDebts();
  std::sort(edges.begin(), edges.end(),
            [](const DebtGraphEdge& e1, const DebtGraphEdge& e2) {
              return e1.debt < e2.debt;
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

    const LayeredGraph blocking_flow =
        LayeredGraph::ConstructBlockingFlow(graph, receiver_id, lender_id);

    uint64_t payer_id;
    for (const LayeredGraphNode& node : blocking_flow) {
      if (node.type == LayeredGraphNodeType::Head) {
        payer_id = node.head.id;
        continue;
      }

      const uint64_t neighbor_id =
          blocking_flow[node.neighbor.neighbor_head_idx].head.id;
      graph.PushFlow(payer_id, neighbor_id, node.neighbor.flow);
    }

    graph.EraseEdge(lender_id, receiver_id);
    simplified_expenses_.PushFlow(receiver_id, lender_id,
                                  blocking_flow.ComputeFlow());
  }
}

}  // namespace debt_simpl
