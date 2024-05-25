#include "src/expense_simplifier.h"

#include <vector>

#include "src/debt_graph.h"

namespace debt_simpl {

ExpenseSimplifier::ExpenseSimplifier(DebtGraph&& graph)
    : graph_(std::move(graph)) {}

DebtGraph ExpenseSimplifier::MinimalTransactions() const {
  return DebtGraph();
}

}  // namespace debt_simpl
