#include "server/src/service.h"

#include "grpcpp/support/status.h"

namespace debt_simpl {

grpc::Status ServiceImpl::Test(grpc::ServerContext* context, const TestReq* req,
                               TestRes* res) {
  std::cout << "Received test req with " << res->msg() << std::endl;
  res->set_msg(req->msg());
  return grpc::Status::OK;
}

}  // namespace debt_simpl
