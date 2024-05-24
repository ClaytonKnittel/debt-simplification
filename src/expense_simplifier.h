#pragma once

#include <vector>

#include "src/debt_graph.h"
#include "src/debts.pb.h"

namespace debt_simpl {

class ExpenseSimplifier {
 public:
  explicit ExpenseSimplifier(DebtGraph graph);

  const std::vector<Transaction> MinimalTransactions() const;

 private:
  DebtGraph graph_;
};

}  // namespace debt_simpl
