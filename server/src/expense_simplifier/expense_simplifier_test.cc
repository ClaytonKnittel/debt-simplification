#include "src/expense_simplifier/expense_simplifier.h"

#include <iostream>
#include <stdint.h>
#include <vector>

#include "google/protobuf/text_format.h"
#include "gtest/gtest.h"

#include "src/expense_simplifier/debt_graph.h"
#include "src/expense_simplifier/utils.h"

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

// Tests that a chain of transactions remains unchanged, since users can't owe
// other users who they didn't originally owe anything to.
TEST_F(TestExpenseSimplifier, TransactionChainUnchanged) {
  ASSERT_OK_AND_DEFINE(ExpenseSimplifier, solver, CreateFromString(R"(
    transactions {
      lender: "a"
      receiver: "b"
      cents: 100
    }
    transactions {
      lender: "b"
      receiver: "c"
      cents: 100
    }
    transactions {
      lender: "c"
      receiver: "d"
      cents: 100
    })"));

  EXPECT_THAT(solver.MinimalTransactions().AmountOwed("a", "b"),
              IsOkAndHolds(100));
  EXPECT_THAT(solver.MinimalTransactions().AmountOwed("b", "c"),
              IsOkAndHolds(100));
  EXPECT_THAT(solver.MinimalTransactions().AmountOwed("c", "d"),
              IsOkAndHolds(100));
}

TEST_F(TestExpenseSimplifier, TriangleReduced) {
  ASSERT_OK_AND_DEFINE(ExpenseSimplifier, solver, CreateFromString(R"(
    transactions {
      lender: "a"
      receiver: "b"
      cents: 100
    }
    transactions {
      lender: "b"
      receiver: "c"
      cents: 100
    }
    transactions {
      lender: "a"
      receiver: "c"
      cents: 100
    })"));

  EXPECT_THAT(solver.MinimalTransactions().AmountOwed("a", "b"),
              IsOkAndHolds(0));
  EXPECT_THAT(solver.MinimalTransactions().AmountOwed("b", "c"),
              IsOkAndHolds(0));
  EXPECT_THAT(solver.MinimalTransactions().AmountOwed("a", "c"),
              IsOkAndHolds(200));
}

TEST_F(TestExpenseSimplifier, LargestDebtorChosenFirst) {
  ASSERT_OK_AND_DEFINE(ExpenseSimplifier, solver, CreateFromString(R"(
    transactions {
      lender: "sink"
      receiver: "largest"
      cents: 1
    }
    transactions {
      lender: "sink"
      receiver: "x"
      cents: 2
    }
    transactions {
      lender: "sink"
      receiver: "y"
      cents: 3
    }
    transactions {
      lender: "x"
      receiver: "largest"
      cents: 2
    }
    transactions {
      lender: "y"
      receiver: "largest"
      cents: 3
    })"));

  EXPECT_THAT(solver.MinimalTransactions().AmountOwed("sink", "largest"),
              IsOkAndHolds(6));
  EXPECT_THAT(solver.MinimalTransactions().AmountOwed("sink", "x"),
              IsOkAndHolds(0));
  EXPECT_THAT(solver.MinimalTransactions().AmountOwed("sink", "y"),
              IsOkAndHolds(0));
  EXPECT_THAT(solver.MinimalTransactions().AmountOwed("x", "largest"),
              IsOkAndHolds(0));
  EXPECT_THAT(solver.MinimalTransactions().AmountOwed("y", "largest"),
              IsOkAndHolds(0));
}

TEST_F(TestExpenseSimplifier, TwoMinimalTransactions) {
  ASSERT_OK_AND_DEFINE(ExpenseSimplifier, solver, CreateFromString(R"(
    transactions {
      lender: "a"
      receiver: "b"
      cents: 1
    }
    transactions {
      lender: "b"
      receiver: "c"
      cents: 2
    }
    transactions {
      lender: "a"
      receiver: "c"
      cents: 2
    })"));

  EXPECT_THAT(solver.MinimalTransactions().AmountOwed("a", "b"),
              IsOkAndHolds(0));
  EXPECT_THAT(solver.MinimalTransactions().AmountOwed("a", "c"),
              IsOkAndHolds(3));
  EXPECT_THAT(solver.MinimalTransactions().AmountOwed("b", "c"),
              IsOkAndHolds(1));
}

}  // namespace debt_simpl
