#include <stdio.h>

#include "google/protobuf/text_format.h"

#include "src/debt_graph.h"
#include "src/debts.pb.h"
#include "src/expense_simplifier.h"
#include "utils.h"

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
      lender: "alice"
      receiver: "bob"
      cents: 100
    }
    transactions {
      lender: "joe"
      receiver: "bob"
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
    })");

  uint64_t alice_id = res.value().FindUserId("alice").value();
  uint64_t eunice_id = res.value().FindUserId("eunice").value();

  debt_simpl::ExpenseSimplifier solver =
      debt_simpl::ExpenseSimplifier(std::move(res.value()));

  const auto layered_graph = solver.ConstructLayeredGraph(eunice_id, alice_id);
  const std::vector expected_result = { debt_simpl::LayeredGraphNode{
      .type = debt_simpl::LayeredGraphNodeType::Head,
      .head = { .id = 0, .level = 0 } } };

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
                  << node.neighbor.capacity << ")" << std::endl;
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
