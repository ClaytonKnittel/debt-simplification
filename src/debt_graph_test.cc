#include "src/debt_graph.h"

#include "absl/strings/str_format.h"
#include "absl/strings/string_view.h"
#include "google/protobuf/text_format.h"
#include "gtest/gtest.h"

#include "src/utils.h"

namespace debt_simpl {

using google::protobuf::TextFormat;
using ::testing::Eq;

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

TEST_F(TestDebtGraph, SingleTransaction) {
  auto status = CreateFromString(
      R"(
      transactions {
        lender: "alice"
        receiver: "bob"
        cents: 100
      }
      )");
  ASSERT_TRUE(status.ok());
  const DebtGraph& graph = status.value();

  EXPECT_THAT(graph.AmountOwed("alice", "bob"), IsOkAndHolds(Eq(100)));
  EXPECT_THAT(graph.AmountOwed("bob", "alice"), IsOkAndHolds(Eq(-100)));
}

TEST_F(TestDebtGraph, DebtsCombine) {
  auto status = CreateFromString(
      R"(
      transactions {
        lender: "alice"
        receiver: "bob"
        cents: 1000
      }
      transactions {
        lender: "alice"
        receiver: "bob"
        cents: 500
      }
      )");
  ASSERT_TRUE(status.ok());
  const DebtGraph& graph = status.value();

  EXPECT_THAT(graph.AmountOwed("alice", "bob"), IsOkAndHolds(Eq(1500)));
  EXPECT_THAT(graph.AmountOwed("bob", "alice"), IsOkAndHolds(Eq(-1500)));
}

TEST_F(TestDebtGraph, DebtAndCreditCancel) {
  auto status = CreateFromString(
      R"(
      transactions {
        lender: "alice"
        receiver: "bob"
        cents: 200
      }
      transactions {
        lender: "bob"
        receiver: "alice"
        cents: 150
      }
      )");
  ASSERT_TRUE(status.ok());
  const DebtGraph& graph = status.value();

  EXPECT_THAT(graph.AmountOwed("alice", "bob"), IsOkAndHolds(Eq(50)));
  EXPECT_THAT(graph.AmountOwed("bob", "alice"), IsOkAndHolds(Eq(-50)));
}

}  // namespace debt_simpl
