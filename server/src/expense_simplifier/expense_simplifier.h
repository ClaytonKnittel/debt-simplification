#pragma once

#include <vector>

#include "src/expense_simplifier/debt_graph.h"

namespace debt_simpl {

class ExpenseSimplifier {
  friend class TestExpenseSimplifier;

 public:
  explicit ExpenseSimplifier(DebtGraph&& graph);

  const DebtGraph& MinimalTransactions() const;

 private:
  void BuildMinimalTransactions(AugmentedDebtGraph&& graph);

  DebtGraph simplified_expenses_;
};

}  // namespace debt_simpl
