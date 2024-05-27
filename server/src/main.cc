#include <stdio.h>

#include "google/protobuf/text_format.h"

#include "proto/debts.pb.h"
#include "src/expense_simplifier/debt_graph.h"
#include "src/expense_simplifier/expense_simplifier.h"
#include "src/expense_simplifier/layered_graph.h"
#include "src/expense_simplifier/utils.h"

using google::protobuf::TextFormat;

absl::StatusOr<debt_simpl::DebtGraph> CreateFromString(
    absl::string_view debt_list_proto) {
  debt_simpl::DebtList debt_list;
  if (!TextFormat::ParseFromString(debt_list_proto, &debt_list)) {
    return absl::InternalError(absl::StrFormat(
        "Failed to construct DebtList proto from string %s", debt_list_proto));
  }

  return debt_simpl::DebtGraph::BuildFromProto(debt_list);
}

int main() {
  absl::StatusOr<debt_simpl::DebtGraph> res = CreateFromString(R"(
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
      cents: 10
    }
    transactions {
      lender: "f"
      receiver: "e"
      cents: 1
    })");

  uint64_t a_id = res.value().FindUserId("a").value();
  uint64_t f_id = res.value().FindUserId("f").value();
  std::cout << "going from " << a_id << " to " << f_id << std::endl;

  std::cout << "a: " << res.value().FindUserId("a").value() << std::endl;
  std::cout << "b: " << res.value().FindUserId("b").value() << std::endl;
  std::cout << "c: " << res.value().FindUserId("c").value() << std::endl;
  std::cout << "d: " << res.value().FindUserId("d").value() << std::endl;
  std::cout << "e: " << res.value().FindUserId("e").value() << std::endl;
  std::cout << "f: " << res.value().FindUserId("f").value() << std::endl;

  const auto layered_graph =
      debt_simpl::LayeredGraph::ConstructBlockingFlow(res.value(), a_id, f_id);

  std::cout << "layered graph:" << std::endl;
  for (const auto& node : layered_graph) {
    switch (node.type) {
      case debt_simpl::LayeredGraphNodeType::Head: {
        std::cout << "Head: " << node.head.id << " (" << node.head.level << ")"
                  << std::endl;
        break;
      }
      case debt_simpl::LayeredGraphNodeType::Neighbor: {
        std::cout << "Neighbor: " << node.neighbor.neighbor_head_idx << " ("
                  << node.neighbor.flow << " of " << node.neighbor.capacity
                  << ")" << std::endl;
        break;
      }
      case debt_simpl::LayeredGraphNodeType::Tombstone: {
        std::cout << "Tombstone" << std::endl;
        break;
      }
    }
  }

  return 0;
}
