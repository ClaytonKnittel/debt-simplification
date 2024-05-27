#include <memory>
#include <stdio.h>

#include "grpcpp/server_builder.h"
#include "modules/httplib/httplib.h"

#include "src/service.h"

int main() {
  const std::string addr = "0.0.0.0:3000";
  debt_simpl::ServiceImpl service;

  grpc::ServerBuilder builder;
  builder.AddListeningPort(addr, grpc::InsecureServerCredentials());
  builder.RegisterService(&service);
  std::unique_ptr<grpc::Server> server(builder.BuildAndStart());

  std::cout << "Server listening on " << addr << std::endl;
  server->Wait();

  return 0;
}
