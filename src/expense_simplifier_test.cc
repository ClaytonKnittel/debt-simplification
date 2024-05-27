#include "src/expense_simplifier.h"

#include <iostream>
#include <stdint.h>
#include <vector>

#include "google/protobuf/text_format.h"
#include "gtest/gtest.h"

#include "src/debt_graph.h"
#include "src/utils.h"

namespace debt_simpl {

using google::protobuf::TextFormat;

class TestExpenseSimplifier : public ::testing::Test {
 protected:
  absl::StatusOr<ExpenseSimplifier> CreateFromString(
      absl::string_view debt_list_proto) {
    DebtList debt_list;
    if (!TextFormat::ParseFromString(debt_list_proto, &debt_list)) {
      return absl::InternalError(
          absl::StrFormat("Failed to construct DebtList proto from string %s",
                          debt_list_proto));
    }

    DEFINE_OR_RETURN(DebtGraph, graph, DebtGraph::BuildFromProto(debt_list));

    return ExpenseSimplifier(std::move(graph));
  }
};

TEST_F(TestExpenseSimplifier, SingleTransaction) {
  ASSERT_OK_AND_DEFINE(ExpenseSimplifier, solver, CreateFromString(R"(
    transactions {
      lender: "alice"
      receiver: "bob"
      cents: 100
    })"));

  EXPECT_THAT(solver.MinimalTransactions().AmountOwed("alice", "bob"),
              IsOkAndHolds(100));
  EXPECT_THAT(solver.MinimalTransactions().AmountOwed("bob", "alice"),
              IsOkAndHolds(-100));
}

TEST_F(TestExpenseSimplifier, Empty) {
  ASSERT_OK_AND_DEFINE(ExpenseSimplifier, solver, CreateFromString(""));

  EXPECT_EQ(solver.MinimalTransactions().NumUsers(), 0);
}

}  // namespace debt_simpl
