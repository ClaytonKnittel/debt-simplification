load("@rules_pkg//pkg:mappings.bzl", "pkg_files", "strip_prefix")

package(
  default_visibility = [
    # Public, but Protobuf only visibility.
    "//:__subpackages__",
  ],
)

pkg_files(
  name = "dist_files",
  srcs = glob(["*"]),
  strip_prefix = strip_prefix.from_root(""),
  visibility = ["//pkg:__pkg__"],
)
