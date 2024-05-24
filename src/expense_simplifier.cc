#include "src/expense_simplifier.h"

#include <vector>

namespace debt_simpl {

ExpenseSimplifier::ExpenseSimplifier(DebtGraph graph)
    : graph_(std::move(graph)) {}

const std::vector<Transaction> ExpenseSimplifier::MinimalTransactions() const {
  return std::vector<Transaction>();
}

}  // namespace debt_simpl
