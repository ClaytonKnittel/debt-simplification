#pragma once

#include <vector>

#include "src/debt_graph.h"
#include "src/debts.pb.h"

namespace debt_simpl {

class ExpenseSimplifier {
 public:
  explicit ExpenseSimplifier(DebtGraph&& graph);

  DebtGraph MinimalTransactions() const;

 private:
  AugmentedDebtGraph graph_;
};

}  // namespace debt_simpl
