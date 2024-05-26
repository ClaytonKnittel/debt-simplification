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

}  // namespace debt_simpl
