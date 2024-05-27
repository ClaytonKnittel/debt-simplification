#include <cstdint>
#include <memory>
#include <string>

#include "absl/status/statusor.h"

namespace httplib {
class Server;
}

class StaticFileServer {
 public:
  ~StaticFileServer();

  StaticFileServer(StaticFileServer&&) = default;
  StaticFileServer& operator=(StaticFileServer&&) = default;

  static absl::StatusOr<StaticFileServer> New(const std::string& dir);

  bool Listen(const std::string& addr, uint16_t port);

 private:
  StaticFileServer();

  std::unique_ptr<httplib::Server> server_;
};
