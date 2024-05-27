#include "src/static_file_server.h"

#include <cstdint>
#include <memory>
#include <string>

#include "absl/status/statusor.h"
#include "modules/httplib/httplib.h"

StaticFileServer::~StaticFileServer() {}

// static
absl::StatusOr<StaticFileServer> StaticFileServer::New(const std::string& dir) {
  StaticFileServer fs;
  if (!fs.server_->set_mount_point("/", dir)) {
    return absl::InternalError(
        "No such directory \"../client/dist/dev/static\"");
  }

  return std::move(fs);
}

bool StaticFileServer::Listen(const std::string& addr, uint16_t port) {
  std::cout << "Static file server listening on " << addr << ":" << port
            << std::endl;
  return server_->listen(addr, port);
}

StaticFileServer::StaticFileServer()
    : server_(std::make_unique<httplib::Server>()) {}
