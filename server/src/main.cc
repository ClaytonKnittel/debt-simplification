#include <cstdint>
#include <memory>
#include <stdio.h>

#include "absl/strings/str_cat.h"
#include "grpcpp/server_builder.h"

#include "server/src/service.h"
#include "server/src/static_file_server.h"

std::unique_ptr<grpc::Server> MakeRpcServer(const std::string& addr,
                                            uint16_t port) {
  debt_simpl::ServiceImpl service;

  grpc::ServerBuilder builder;
  builder.AddListeningPort(absl::StrCat(addr, ":", port),
                           grpc::InsecureServerCredentials());
  builder.RegisterService(&service);

  auto server = builder.BuildAndStart();
  std::cout << "RPC server listening on " << addr << std::endl;

  return server;
}

int main() {
  const std::string addr = "0.0.0.0";
  const uint16_t sfs_port = 3000;
  const uint16_t rpc_port = 3001;

  auto file_server = StaticFileServer::New("../client/dist/dev/static");
  auto rpc_server = MakeRpcServer(addr, rpc_port);

  file_server->Listen(addr, sfs_port);
  rpc_server->Wait();

  return 0;
}
