#include "src/debt_graph.h"

DebtGraphNode::DebtGraphNode(uint64_t id) : id_(id) {}

void DebtGraphNode::AddDebt(uint64_t owee_id, Cents amount) {
  debts_[owee_id] += amount;
}
