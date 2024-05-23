#include "src/debt_graph.h"

#include "google/protobuf/text_format.h"
#include "gtest/gtest.h"

#include "src/utils.h"

namespace debt_simpl {

using google::protobuf::TextFormat;
using ::testing::Eq;

TEST(TestDebtGraph, TestSimple) {
  DebtList debt_list;
  ASSERT_TRUE(TextFormat::ParseFromString(R"(
    transactions {
      lender: "alice"
      receiver: "bob"
      cents: 100
    })",
                                          &debt_list));

  auto status = DebtGraph::BuildFromProto(debt_list);
  ASSERT_TRUE(status.ok());
  DebtGraph& graph = status.value();

  EXPECT_THAT(graph.AmountOwed("alice", "bob"), IsOkAndHolds(Eq(100)));
  EXPECT_THAT(graph.AmountOwed("bob", "alice"), IsOkAndHolds(Eq(-100)));
}

}  // namespace debt_simpl
