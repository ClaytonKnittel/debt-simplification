#pragma once

#include "proto/service.grpc.pb.h"

namespace debt_simpl {

class ServiceImpl : public DebtSimplifier::Service {};

}  // namespace debt_simpl
