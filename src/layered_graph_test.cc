#include "src/layered_graph.h"

#include <iostream>
#include <stdint.h>
#include <vector>

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

class TestLayeredGraph : public ::testing::Test {
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

class TestBlockingFlow : public TestLayeredGraph {};

TEST_F(TestBlockingFlow, TestSingleTransaction) {
  ASSERT_OK_AND_DEFINE(DebtGraph, graph, CreateFromString(R"(
    transactions {
      lender: "alice"
      receiver: "bob"
      cents: 100
    })"));

  ASSERT_OK_AND_DEFINE(uint64_t, alice_id, graph.FindUserId("alice"));
  ASSERT_OK_AND_DEFINE(uint64_t, bob_id, graph.FindUserId("bob"));

  AugmentedDebtGraph augmented_graph = std::move(graph);

  const auto layered_graph =
      LayeredGraph::ConstructBlockingFlow(augmented_graph, bob_id, alice_id);
  const std::vector expected_result = {
    LayeredGraphNode{ .type = LayeredGraphNodeType::Head,
                      .head = { .id = bob_id, .level = 0 } },
    LayeredGraphNode{
        .type = LayeredGraphNodeType::Neighbor,
        .neighbor = { .neighbor_head_idx = 2, .capacity = 100, .flow = 100 } },
    LayeredGraphNode{ .type = LayeredGraphNodeType::Head,
                      .head = { .id = alice_id, .level = 1 } }
  };
  EXPECT_THAT(layered_graph.NodeList(), ContainerEq(expected_result));
}

TEST_F(TestBlockingFlow, TestNoPath) {
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

  AugmentedDebtGraph augmented_graph = std::move(graph);

  const auto layered_graph =
      LayeredGraph::ConstructBlockingFlow(augmented_graph, joe_id, alice_id);
  EXPECT_THAT(layered_graph.NodeList(),
              ContainerEq(std::vector<LayeredGraphNode>()));
}

TEST_F(TestBlockingFlow, TestTwoPaths) {
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

  AugmentedDebtGraph augmented_graph = std::move(graph);

  const auto layered_graph =
      LayeredGraph::ConstructBlockingFlow(augmented_graph, eunice_id, bob_id);
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

TEST_F(TestBlockingFlow, TestPrunePaths) {
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

  AugmentedDebtGraph augmented_graph = std::move(graph);

  const auto layered_graph =
      LayeredGraph::ConstructBlockingFlow(augmented_graph, eunice_id, bob_id);
  ASSERT_EQ(layered_graph.size(), 2 + 2 + 1);

  EXPECT_EQ(layered_graph[0],
            (LayeredGraphNode{ .type = LayeredGraphNodeType::Head,
                               .head = { .id = eunice_id, .level = 0 } }));
  EXPECT_EQ(layered_graph[1],
            (LayeredGraphNode{ .type = LayeredGraphNodeType::Neighbor,
                               .neighbor = {
                                   .neighbor_head_idx = 2,
                                   .capacity = 100,
                                   .flow = 50,
                               } }));

  EXPECT_EQ(layered_graph[2],
            (LayeredGraphNode{ .type = LayeredGraphNodeType::Head,
                               .head = { .id = alice_id, .level = 1 } }));
  EXPECT_EQ(layered_graph[3],
            (LayeredGraphNode{ .type = LayeredGraphNodeType::Neighbor,
                               .neighbor = {
                                   .neighbor_head_idx = 4,
                                   .capacity = 50,
                                   .flow = 50,
                               } }));

  EXPECT_EQ(layered_graph[4],
            (LayeredGraphNode{ .type = LayeredGraphNodeType::Head,
                               .head = { .id = bob_id, .level = 2 } }));
}

TEST_F(TestBlockingFlow, TestMultipleFlowsPossible) {
  // This is a visual representation of the flow network (from left to right):
  //
  //     _2_ b
  //   /       \_1_
  // a          _1_ e
  //   \_3_   /       \_1_
  //        c          _9_ f
  //          \_1_   /
  //               d
  ASSERT_OK_AND_DEFINE(DebtGraph, graph, CreateFromString(R"(
    transactions {
      lender: "b"
      receiver: "a"
      cents: 2
    }
    transactions {
      lender: "c"
      receiver: "a"
      cents: 3
    }
    transactions {
      lender: "d"
      receiver: "c"
      cents: 1
    }
    transactions {
      lender: "e"
      receiver: "c"
      cents: 1
    }
    transactions {
      lender: "e"
      receiver: "b"
      cents: 1
    }
    transactions {
      lender: "f"
      receiver: "d"
      cents: 9
    }
    transactions {
      lender: "f"
      receiver: "e"
      cents: 1
    })"));

  ASSERT_OK_AND_DEFINE(uint64_t, a_id, graph.FindUserId("a"));
  ASSERT_OK_AND_DEFINE(uint64_t, b_id, graph.FindUserId("b"));
  ASSERT_OK_AND_DEFINE(uint64_t, c_id, graph.FindUserId("c"));
  ASSERT_OK_AND_DEFINE(uint64_t, d_id, graph.FindUserId("d"));
  ASSERT_OK_AND_DEFINE(uint64_t, e_id, graph.FindUserId("e"));
  ASSERT_OK_AND_DEFINE(uint64_t, f_id, graph.FindUserId("f"));

  AugmentedDebtGraph augmented_graph = std::move(graph);

  const auto layered_graph =
      LayeredGraph::ConstructBlockingFlow(augmented_graph, a_id, f_id);
  ASSERT_EQ(layered_graph.size(), 3 + 2 + 3 + 2 + 2 + 1);

  uint64_t a_idx = UINT64_MAX, b_idx = UINT64_MAX, c_idx = UINT64_MAX,
           d_idx = UINT64_MAX, e_idx = UINT64_MAX, f_idx = UINT64_MAX;
  for (uint64_t i = 0; i < layered_graph.size(); i++) {
    if (layered_graph[i].type == LayeredGraphNodeType::Head) {
      if (layered_graph[i].head.id == a_id) {
        a_idx = i;
      } else if (layered_graph[i].head.id == b_id) {
        b_idx = i;
      } else if (layered_graph[i].head.id == c_id) {
        c_idx = i;
      } else if (layered_graph[i].head.id == d_id) {
        d_idx = i;
      } else if (layered_graph[i].head.id == e_id) {
        e_idx = i;
      } else if (layered_graph[i].head.id == f_id) {
        f_idx = i;
      }
    }
  }
  ASSERT_NE(a_idx, UINT64_MAX);
  ASSERT_NE(b_idx, UINT64_MAX);
  ASSERT_NE(c_idx, UINT64_MAX);
  ASSERT_NE(d_idx, UINT64_MAX);
  ASSERT_NE(e_idx, UINT64_MAX);
  ASSERT_NE(f_idx, UINT64_MAX);

  uint64_t c_to_d_edge_idx = UINT64_MAX;
  for (uint64_t i = c_idx + 1;
       layered_graph[i].type != LayeredGraphNodeType::Head; i++) {
    if (layered_graph[i].neighbor.neighbor_head_idx == d_idx) {
      c_to_d_edge_idx = i;
      break;
    }
  }
  ASSERT_NE(c_to_d_edge_idx, UINT64_MAX);

  EXPECT_EQ(layered_graph[a_idx],
            (LayeredGraphNode{ .type = LayeredGraphNodeType::Head,
                               .head = { .id = a_id, .level = 0 } }));
  EXPECT_EQ(layered_graph[a_idx + 1].type, LayeredGraphNodeType::Neighbor);
  EXPECT_EQ(layered_graph[a_idx + 2].type, LayeredGraphNodeType::Neighbor);

  // There are two possible flow networks:
  //     _1_ b
  //   /       \_1_
  // a          _0_ e
  //   \_1_   /       \_1_
  //        c          _1_ f
  //          \_1_   /
  //               d
  //
  // and
  //     _0_ b
  //   /       \_0_
  // a          _1_ e
  //   \_2_   /       \_1_
  //        c          _1_ f
  //          \_1_   /
  //               d
  //
  // Regardless which one is chosen, the following are true:

  // The flow out of a is 2:
  EXPECT_EQ(layered_graph[a_idx + 1].neighbor.flow +
                layered_graph[a_idx + 2].neighbor.flow,
            2);

  // The flow through e is 1:
  EXPECT_EQ(layered_graph[e_idx + 1].neighbor.flow, 1);

  // The flow from c to d is 1:
  EXPECT_EQ(layered_graph[c_to_d_edge_idx].neighbor.flow, 1);

  // The flow into f is 2:
  EXPECT_EQ(layered_graph[d_idx + 1].neighbor.neighbor_head_idx, f_idx);
  EXPECT_EQ(layered_graph[d_idx + 1].neighbor.flow, 1);
  EXPECT_EQ(layered_graph[e_idx + 1].neighbor.neighbor_head_idx, f_idx);
  EXPECT_EQ(layered_graph[e_idx + 1].neighbor.flow, 1);
}

}  // namespace debt_simpl
