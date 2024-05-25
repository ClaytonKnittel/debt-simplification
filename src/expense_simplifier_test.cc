#include "src/expense_simplifier.h"

#include <iostream>

#include "gmock/gmock.h"
#include "google/protobuf/text_format.h"
#include "gtest/gtest.h"

#include "src/debt_graph.h"
#include "src/utils.h"

namespace debt_simpl {

using google::protobuf::TextFormat;
using ::testing::AnyOf;
using ::testing::ContainerEq;
using ::testing::Eq;

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

class TestLayeredGraph : public TestExpenseSimplifier {};

TEST_F(TestLayeredGraph, TestSingleTransaction) {
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

TEST_F(TestLayeredGraph, TestNoPath) {
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

TEST_F(TestLayeredGraph, TestTwoPaths) {
  ASSERT_OK_AND_DEFINE(DebtGraph, graph, CreateFromString(R"(
    transactions {
      lender: "bob"
      receiver: "alice"
      cents: 100
    }
    transactions {
      lender: "bob"
      receiver: "joe"
      cents: 100
    }
    transactions {
      lender: "alice"
      receiver: "eunice"
      cents: 100
    }
    transactions {
      lender: "joe"
      receiver: "eunice"
      cents: 100
    })"));

  ASSERT_OK_AND_DEFINE(uint64_t, alice_id, graph.FindUserId("alice"));
  ASSERT_OK_AND_DEFINE(uint64_t, bob_id, graph.FindUserId("bob"));
  ASSERT_OK_AND_DEFINE(uint64_t, joe_id, graph.FindUserId("joe"));
  ASSERT_OK_AND_DEFINE(uint64_t, eunice_id, graph.FindUserId("eunice"));

  ExpenseSimplifier solver(std::move(graph));

  const auto layered_graph = solver.ConstructLayeredGraph(eunice_id, bob_id);
  ASSERT_EQ(layered_graph.size(), 3 + 2 + 2 + 1);

  EXPECT_EQ(layered_graph[0],
            (LayeredGraphNode{ .type = LayeredGraphNodeType::Head,
                               .head = { .id = eunice_id, .level = 0 } }));
  EXPECT_EQ(layered_graph[1].type, LayeredGraphNodeType::Neighbor);
  EXPECT_EQ(layered_graph[2].type, LayeredGraphNodeType::Neighbor);

  EXPECT_THAT(
      layered_graph[3],
      AnyOf(Eq(LayeredGraphNode{ .type = LayeredGraphNodeType::Head,
                                 .head = { .id = alice_id, .level = 1 } }),
            Eq(LayeredGraphNode{ .type = LayeredGraphNodeType::Head,
                                 .head = { .id = joe_id, .level = 1 } })));
  EXPECT_EQ(layered_graph[4].type, LayeredGraphNodeType::Neighbor);

  EXPECT_THAT(
      layered_graph[5],
      AnyOf(Eq(LayeredGraphNode{ .type = LayeredGraphNodeType::Head,
                                 .head = { .id = alice_id, .level = 1 } }),
            Eq(LayeredGraphNode{ .type = LayeredGraphNodeType::Head,
                                 .head = { .id = joe_id, .level = 1 } })));
  EXPECT_EQ(layered_graph[6].type, LayeredGraphNodeType::Neighbor);

  EXPECT_EQ(layered_graph[7],
            (LayeredGraphNode{ .type = LayeredGraphNodeType::Head,
                               .head = { .id = bob_id, .level = 2 } }));
}

TEST_F(TestLayeredGraph, TestPrunePaths) {
  ASSERT_OK_AND_DEFINE(DebtGraph, graph, CreateFromString(R"(
    transactions {
      lender: "bob"
      receiver: "alice"
      cents: 50
    }
    transactions {
      lender: "farquat"
      receiver: "joe"
      cents: 104
    }
    transactions {
      lender: "alice"
      receiver: "eunice"
      cents: 100
    }
    transactions {
      lender: "joe"
      receiver: "eunice"
      cents: 102
    })"));

  ASSERT_OK_AND_DEFINE(uint64_t, alice_id, graph.FindUserId("alice"));
  ASSERT_OK_AND_DEFINE(uint64_t, bob_id, graph.FindUserId("bob"));
  ASSERT_OK_AND_DEFINE(uint64_t, eunice_id, graph.FindUserId("eunice"));

  ExpenseSimplifier solver(std::move(graph));

  const auto layered_graph = solver.ConstructLayeredGraph(eunice_id, bob_id);
  ASSERT_EQ(layered_graph.size(), 2 + 2 + 1);

  EXPECT_EQ(layered_graph[0],
            (LayeredGraphNode{ .type = LayeredGraphNodeType::Head,
                               .head = { .id = eunice_id, .level = 0 } }));
  EXPECT_EQ(layered_graph[1],
            (LayeredGraphNode{ .type = LayeredGraphNodeType::Neighbor,
                               .neighbor = {
                                   .neighbor_head_idx = 2,
                                   .capacity = 100,
                               } }));

  EXPECT_EQ(layered_graph[2],
            (LayeredGraphNode{ .type = LayeredGraphNodeType::Head,
                               .head = { .id = alice_id, .level = 1 } }));
  EXPECT_EQ(layered_graph[3],
            (LayeredGraphNode{ .type = LayeredGraphNodeType::Neighbor,
                               .neighbor = {
                                   .neighbor_head_idx = 4,
                                   .capacity = 50,
                               } }));

  EXPECT_EQ(layered_graph[4],
            (LayeredGraphNode{ .type = LayeredGraphNodeType::Head,
                               .head = { .id = bob_id, .level = 2 } }));
}

}  // namespace debt_simpl
