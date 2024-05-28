#include "server/src/expense_simplifier/debt_graph.h"

#include <cstdint>

#include "absl/strings/str_format.h"
#include "absl/strings/string_view.h"
#include "gmock/gmock.h"
#include "google/protobuf/text_format.h"
#include "gtest/gtest.h"

#include "server/src/expense_simplifier/utils.h"

namespace debt_simpl {

using google::protobuf::TextFormat;
using ::testing::Not;

class TestDebtGraph : public ::testing::Test {
 protected:
  absl::StatusOr<DebtGraph> CreateFromString(
      absl::string_view debt_list_proto) {
    DebtList debt_list;
    if (!TextFormat::ParseFromString(debt_list_proto, &debt_list)) {
      return absl::InternalError(
          absl::StrFormat("Failed to construct DebtList proto from string %s",
                          debt_list_proto));
    }

    return DebtGraph::BuildFromProto(debt_list);
  }
};

class TestAugmentedDebtGraph : public TestDebtGraph {};

TEST_F(TestDebtGraph, SingleTransaction) {
  DebtGraph graph;
  ASSERT_OK_AND_ASSIGN(graph, CreateFromString(R"(
    transactions {
      lender: "alice"
      receiver: "bob"
      cents: 100
    })"));

  EXPECT_THAT(graph.AmountOwed("alice", "bob"), IsOkAndHolds(100));
  EXPECT_THAT(graph.AmountOwed("bob", "alice"), IsOkAndHolds(-100));

  EXPECT_THAT(graph.TotalDebt("alice"), IsOkAndHolds(-100));
  EXPECT_THAT(graph.TotalDebt("bob"), IsOkAndHolds(100));
}

TEST_F(TestDebtGraph, DebtsCombine) {
  DebtGraph graph;
  ASSERT_OK_AND_ASSIGN(graph, CreateFromString(R"(
    transactions {
      lender: "alice"
      receiver: "bob"
      cents: 1000
    }
    transactions {
      lender: "alice"
      receiver: "bob"
      cents: 500
    })"));

  EXPECT_THAT(graph.AmountOwed("alice", "bob"), IsOkAndHolds(1500));
  EXPECT_THAT(graph.AmountOwed("bob", "alice"), IsOkAndHolds(-1500));

  EXPECT_THAT(graph.TotalDebt("alice"), IsOkAndHolds(-1500));
  EXPECT_THAT(graph.TotalDebt("bob"), IsOkAndHolds(1500));
}

TEST_F(TestDebtGraph, DebtAndCreditCancel) {
  DebtGraph graph;
  ASSERT_OK_AND_ASSIGN(graph, CreateFromString(R"(
    transactions {
      lender: "alice"
      receiver: "bob"
      cents: 200
    }
    transactions {
      lender: "bob"
      receiver: "alice"
      cents: 150
    })"));

  EXPECT_THAT(graph.AmountOwed("alice", "bob"), IsOkAndHolds(50));
  EXPECT_THAT(graph.AmountOwed("bob", "alice"), IsOkAndHolds(-50));

  EXPECT_THAT(graph.TotalDebt("alice"), IsOkAndHolds(-50));
  EXPECT_THAT(graph.TotalDebt("bob"), IsOkAndHolds(50));
}

TEST_F(TestDebtGraph, UnknownUser) {
  DebtGraph graph;
  ASSERT_OK_AND_ASSIGN(graph, CreateFromString(R"(
    transactions {
      lender: "x"
      receiver: "y"
      cents: 200
    })"));

  EXPECT_THAT(graph.AmountOwed("x", "y"), IsOkAndHolds(200));
  EXPECT_THAT(graph.AmountOwed("x", "z"), Not(IsOk()));
  EXPECT_THAT(graph.AmountOwed("z", "y"), Not(IsOk()));

  EXPECT_THAT(graph.TotalDebt("z"), Not(IsOk()));
}

TEST_F(TestDebtGraph, TwoTransactions) {
  DebtGraph graph;
  ASSERT_OK_AND_ASSIGN(graph, CreateFromString(R"(
    transactions {
      lender: "alice"
      receiver: "bob"
      cents: 100
    }
    transactions {
      lender: "bob"
      receiver: "joe"
      cents: 50
    })"));

  EXPECT_THAT(graph.AmountOwed("alice", "bob"), IsOkAndHolds(100));
  EXPECT_THAT(graph.AmountOwed("bob", "joe"), IsOkAndHolds(50));
  EXPECT_THAT(graph.AmountOwed("alice", "joe"), IsOkAndHolds(0));

  EXPECT_THAT(graph.TotalDebt("alice"), IsOkAndHolds(-100));
  EXPECT_THAT(graph.TotalDebt("bob"), IsOkAndHolds(50));
  EXPECT_THAT(graph.TotalDebt("joe"), IsOkAndHolds(50));
}

TEST_F(TestAugmentedDebtGraph, TotalDebt) {
  DebtGraph graph;
  ASSERT_OK_AND_ASSIGN(graph, CreateFromString(R"(
    transactions {
      lender: "alice"
      receiver: "bob"
      cents: 100
    })"));

  ASSERT_OK_AND_DEFINE(uint64_t, alice_id, graph.FindUserId("alice"));
  ASSERT_OK_AND_DEFINE(uint64_t, bob_id, graph.FindUserId("bob"));

  AugmentedDebtGraph augmented_graph = std::move(graph);

  EXPECT_EQ(augmented_graph.Debt(alice_id, bob_id), 0);
  EXPECT_EQ(augmented_graph.Debt(bob_id, alice_id), 100);

  EXPECT_EQ(augmented_graph.TotalDebt(alice_id), -100);
  EXPECT_EQ(augmented_graph.TotalDebt(bob_id), 100);
}

TEST_F(TestAugmentedDebtGraph, DebtFlow) {
  DebtGraph graph;
  ASSERT_OK_AND_ASSIGN(graph, CreateFromString(R"(
    transactions {
      lender: "alice"
      receiver: "bob"
      cents: 100
    })"));

  ASSERT_OK_AND_DEFINE(uint64_t, alice_id, graph.FindUserId("alice"));
  ASSERT_OK_AND_DEFINE(uint64_t, bob_id, graph.FindUserId("bob"));

  AugmentedDebtGraph augmented_graph = std::move(graph);
  augmented_graph.PushFlow(bob_id, alice_id, 10);

  EXPECT_EQ(augmented_graph.TotalDebt(alice_id), -90);
  EXPECT_EQ(augmented_graph.TotalDebt(bob_id), 90);
}

}  // namespace debt_simpl
