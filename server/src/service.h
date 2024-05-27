#pragma once

#include "grpcpp/support/status.h"

#include "proto/service.grpc.pb.h"

namespace debt_simpl {

class ServiceImpl : public DebtSimplifier::Service {
 public:
  grpc::Status Test(grpc::ServerContext*, const TestReq*, TestRes*) override;
};

}  // namespace debt_simpl
