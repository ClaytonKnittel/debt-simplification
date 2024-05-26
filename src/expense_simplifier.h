#pragma once

#include <vector>

#include "src/debt_graph.h"

namespace debt_simpl {

class ExpenseSimplifier {
  friend class TestExpenseSimplifier;

 public:
  explicit ExpenseSimplifier(DebtGraph&& graph);

  const DebtGraph& MinimalTransactions() const;

 protected:
 private:
  DebtGraph simplified_expenses_;
};

}  // namespace debt_simpl
