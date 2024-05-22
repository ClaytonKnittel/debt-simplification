#pragma once

#include "absl/container/flat_hash_map.h"

class DebtGraph {
 private:
  absl::flat_hash_map<int, int> guys;
};
