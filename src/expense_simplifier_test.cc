#include "src/expense_simplifier.h"

#include <iostream>

#include "gmock/gmock.h"
#include "google/protobuf/text_format.h"
#include "gtest/gtest.h"

#include "src/debt_graph.h"
#include "src/utils.h"

namespace debt_simpl {

using google::protobuf::TextFormat;
using ::testing::ContainerEq;

class TestExpenseSimplifier : public ::testing::Test {
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

TEST_F(TestExpenseSimplifier, TestSingleTransaction) {
  ASSERT_OK_AND_DEFINE(DebtGraph, graph, CreateFromString(R"(
    transactions {
      lender: "alice"
      receiver: "bob"
      cents: 100
    })"));

  ASSERT_OK_AND_DEFINE(uint64_t, alice_id, graph.FindUserId("alice"));
  ASSERT_OK_AND_DEFINE(uint64_t, bob_id, graph.FindUserId("bob"));

  ExpenseSimplifier solver(std::move(graph));

  const auto layered_graph = solver.ConstructLayeredGraph(bob_id, alice_id);
  const std::vector expected_result = {
    LayeredGraphNode{ .type = LayeredGraphNodeType::Head,
                      .head = { .id = bob_id, .level = 0 } },
    LayeredGraphNode{ .type = LayeredGraphNodeType::Neighbor,
                      .neighbor = { .neighbor_head_idx = 2, .capacity = 100 } },
    LayeredGraphNode{ .type = LayeredGraphNodeType::Head,
                      .head = { .id = alice_id, .level = 1 } }
  };
  EXPECT_THAT(layered_graph, ContainerEq(expected_result));
}

TEST_F(TestExpenseSimplifier, TestNoPath) {
  ASSERT_OK_AND_DEFINE(DebtGraph, graph, CreateFromString(R"(
    transactions {
      lender: "alice"
      receiver: "bob"
      cents: 100
    }
    transactions {
      lender: "joe"
      receiver: "bob"
      cents: 100
    })"));

  ASSERT_OK_AND_DEFINE(uint64_t, alice_id, graph.FindUserId("alice"));
  ASSERT_OK_AND_DEFINE(uint64_t, joe_id, graph.FindUserId("joe"));

  ExpenseSimplifier solver(std::move(graph));

  const auto layered_graph = solver.ConstructLayeredGraph(joe_id, alice_id);
  EXPECT_THAT(layered_graph, ContainerEq(std::vector<LayeredGraphNode>()));
}

TEST_F(TestExpenseSimplifier, TestTwoPaths) {
  ASSERT_OK_AND_DEFINE(DebtGraph, graph, CreateFromString(R"(
    transactions {
      lender: "alice"
      receiver: "bob"
      cents: 100
    }
    transactions {
      lender: "alice"
      receiver: "joe"
      cents: 100
    }
    transactions {
      lender: "joe"
      receiver: "bob"
      cents: 100
    })"));

  ASSERT_OK_AND_DEFINE(uint64_t, alice_id, graph.FindUserId("alice"));
  ASSERT_OK_AND_DEFINE(uint64_t, bob_id, graph.FindUserId("bob"));
  ASSERT_OK_AND_DEFINE(uint64_t, joe_id, graph.FindUserId("joe"));

  ExpenseSimplifier solver(std::move(graph));

  const auto layered_graph = solver.ConstructLayeredGraph(bob_id, alice_id);
  const std::vector expected_result = {
    LayeredGraphNode{ .type = LayeredGraphNodeType::Head,
                      .head = { .id = bob_id, .level = 0 } },
    LayeredGraphNode{ .type = LayeredGraphNodeType::Neighbor,
                      .neighbor = { .neighbor_head_idx = 3, .capacity = 100 } },
    LayeredGraphNode{ .type = LayeredGraphNodeType::Head,
                      .head = { .id = alice_id, .level = 1 } }
  };
  EXPECT_THAT(layered_graph, ContainerEq(expected_result));
}

}  // namespace debt_simpl
