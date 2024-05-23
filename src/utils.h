#pragma once

#include "absl/status/status.h"

#define RETURN_IF_ERROR(expr)      \
  do {                             \
    absl::Status _status = (expr); \
    if (!_status.ok())             \
      return _status;              \
  } while (0)
