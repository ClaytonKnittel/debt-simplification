#include "src/expense_simplifier.h"

#include "src/debt_graph.h"

namespace debt_simpl {

ExpenseSimplifier::ExpenseSimplifier(DebtGraph&& graph) {
  AugmentedDebtGraph augmented_graph(std::move(graph));
}

const DebtGraph& ExpenseSimplifier::MinimalTransactions() const {
  return simplified_expenses_;
}

}  // namespace debt_simpl
